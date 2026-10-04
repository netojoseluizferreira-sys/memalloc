// Implementação do alocador de memória simplificado.
// Gerencia um buffer estático como um mini-heap, dividido em blocos
// de metadados que formam uma lista encadeada.
#include "memalloc.h"
#include <stdio.h>
#include <stdlib.h>

// Estrutura de metadados que precede cada bloco de dados no buffer.
// Fica imediatamente antes dos bytes que o usuário recebe.
typedef struct Bloco {
    size_t tamanho;       // quantidade de bytes de DADOS disponíveis
    int livre;            // 1 = disponível, 0 = ocupado
    struct Bloco *prox;   // próximo bloco na lista
} Bloco;

// Estado global do alocador.
// static = só visível neste arquivo. Ninguém de fora mexe.
static unsigned char *g_buffer = NULL;         // início do buffer
static Bloco *g_primeiro_bloco = NULL;         // entrada da lista de blocos
static size_t g_tamanho_total = 0;             // tamanho total (para stats)

// Inicializa o alocador com o buffer padrão.
// Idempotente: chamar de novo não faz nada (evita vazar memória).
// Retorna 1 em sucesso, 0 em falha.
int memalloc_iniciar(void) {
    // Se já foi iniciado, considera sucesso sem reiniciar
    if (g_buffer != NULL) {
        return 1;
    }

    // Pede o buffer ao sistema
    g_buffer = (unsigned char *)malloc(MEMALLOC_TAMANHO_PADRAO);
    if (g_buffer == NULL) {
        return 0;  // falha: sem memória
    }

    // O início do buffer vira o primeiro bloco de metadados.
    // Cast: reinterpreta os bytes crus como uma struct Bloco.
    g_primeiro_bloco = (Bloco *)g_buffer;

    // Esse bloco cobre TODO o buffer, exceto o próprio cabeçalho.
    g_primeiro_bloco->tamanho = MEMALLOC_TAMANHO_PADRAO - sizeof(Bloco);
    g_primeiro_bloco->livre = 1;      // começa livre
    g_primeiro_bloco->prox = NULL;    // único bloco

    g_tamanho_total = MEMALLOC_TAMANHO_PADRAO;
    return 1;
}

// Aloca 'tamanho' bytes do buffer.
// Percorre a lista buscando bloco livre com espaço suficiente.
// Se achar, divide o bloco em dois (um ocupado do tamanho pedido,
// outro livre com o resto) e devolve ponteiro para os dados.
// Retorna NULL se não houver espaço.
void *memalloc_alocar(size_t tamanho) {
    // Passo 1: percorre a lista buscando bloco livre e grande o bastante
    Bloco *atual = g_primeiro_bloco;

    while (atual != NULL) {
        if (atual->livre && atual->tamanho >= tamanho) {
            break;  // achou
        }
        atual = atual->prox;
    }

    // Não achou bloco disponível
    if (atual == NULL) return NULL;

    // Passo 2: calcula quanto sobra se alocar o pedido
    size_t sobra = atual->tamanho - tamanho;

    // Passo 3: se sobrar espaço para outro bloco (cabeçalho + dados),
    // divide em dois. Senão, deixa o bloco com o tamanho atual
    // (desperdício interno, aceitável).
    if (sobra > sizeof(Bloco)) {
        // O novo bloco começa depois do cabeçalho + dados do bloco atual.
        // Aritmética em bytes: cast para unsigned char * para que +N
        // avance exatamente N bytes (não N * sizeof(Bloco)).
        Bloco *novo = (Bloco *)((unsigned char *)atual + sizeof(Bloco) + tamanho);

        novo->tamanho = sobra - sizeof(Bloco);
        novo->livre = 1;
        novo->prox = atual->prox;  // encadeia após o atual

        atual->tamanho = tamanho;  // atual fica com o tamanho pedido
        atual->prox = novo;         // atual aponta para o novo
    }

    // Passo 4: marca como ocupado (sempre, mesmo se não dividiu)
    atual->livre = 0;

    // Passo 5: devolve ponteiro para os DADOS (depois do cabeçalho)
    return (void *)((unsigned char *)atual + sizeof(Bloco));
}

// Libera o bloco previamente alocado por memalloc_alocar.
// Marca como livre e funde com blocos vizinhos livres (coalescência),
// evitando fragmentação.
void memalloc_liberar(void *ptr) {
    Bloco *anterior = g_primeiro_bloco;
    if (ptr == NULL) return;

    // Acha o bloco a partir do ponteiro de dados.
    // Aritmética inversa do alocar: ptr - sizeof(Bloco) = endereço do bloco.
    Bloco *b = (Bloco *)((unsigned char *)ptr - sizeof(Bloco));

    // Marca o bloco como livre
    b->livre = 1;

    // Coalescência com PRÓXIMO: enquanto o próximo estiver livre,
    // absorve ele (soma cabeçalho + dados) e pula na lista.
    // Faz isso ANTES da coalescência com anterior porque, se
    // fundíssemos com o anterior primeiro, perderíamos a referência
    // para o próximo (o bloco b deixaria de existir).
    while (b->prox != NULL && b->prox->livre) {
        b->tamanho += sizeof(Bloco) + b->prox->tamanho;
        b->prox = b->prox->prox;
    }

    // Se b é o primeiro bloco, não tem anterior. Nada a fazer.
    if (b == anterior) return;

    // Percorre a lista até achar quem aponta para b (o bloco anterior)
    while (anterior->prox != b) {
        anterior = anterior->prox;
    }

    // Coalescência com ANTERIOR: se o anterior também está livre,
    // ele absorve b (soma cabeçalho de b + dados de b) e pula para
    // o próximo de b.
    if (anterior->livre == 1) {
        anterior->tamanho += sizeof(Bloco) + b->tamanho;
        anterior->prox = b->prox;
    }
}

// Imprime estatísticas de uso: total, livre, ocupado, blocos.
// 'livre' e 'ocupado' contam apenas os bytes de DADOS (excluem cabeçalhos).
// A diferença entre Total e (Livre + Ocupado) é o overhead dos metadados.
void memalloc_estatisticas(void) {
    Bloco *atual = g_primeiro_bloco;
    size_t livre = 0;
    size_t ocupado = 0;
    int blocos = 0;

    // Percorre a lista acumulando tamanhos
    while (atual != NULL) {
        if (atual->livre == 1) {
            livre += atual->tamanho;
        } else {
            ocupado += atual->tamanho;
        }
        blocos++;
        atual = atual->prox;
    }

    printf("=== Estatísticas do memalloc ===\n");
    printf("Total: %zu bytes\n", g_tamanho_total);
    printf("Livre: %zu bytes\n", livre);
    printf("Ocupado: %zu bytes\n", ocupado);
    printf("Blocos: %d\n", blocos);
}