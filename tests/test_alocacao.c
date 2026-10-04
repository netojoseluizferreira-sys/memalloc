// Testes de alocação do memalloc.
// Verifica que memalloc_alocar devolve ponteiros válidos e distintos,
// que a memória é usável, e que pedidos grandes demais falham.
#include "memalloc.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    // Inicializa o alocador — se falhar, aborta
    assert(memalloc_iniciar() == 1);

    // Teste 1: alocar retorna ponteiro não-NULL quando há espaço
    void *a = memalloc_alocar(100);
    assert(a != NULL);

    // Teste 2: dois ponteiros alocados apontam para endereços diferentes
    void *b = memalloc_alocar(50);
    assert(b != NULL);
    assert(a != b);

    // Teste 3: os ponteiros estão dentro do buffer gerenciado
    // (não podemos verificar isso diretamente sem expor o buffer,
    // mas podemos garantir que são diferentes do cabeçalho)
    assert((unsigned char *)a + 100 <= (unsigned char *)b);

    // Teste 4: a memória alocada é usável (escrever e ler)
    int *ints = (int *)memalloc_alocar(10 * sizeof(int));
    assert(ints != NULL);

    for (int i = 0; i < 10; i++) {
        ints[i] = i * 7;
    }
    for (int i = 0; i < 10; i++) {
        assert(ints[i] == i * 7);
    }

    // Teste 5: alocar mais bytes do que o buffer tem retorna NULL
    void *grande = memalloc_alocar(MEMALLOC_TAMANHO_PADRAO * 10);
    assert(grande == NULL);

    // Libera tudo (não é o foco deste teste, mas evita leak de blocos internos)
    memalloc_liberar(a);
    memalloc_liberar(b);
    memalloc_liberar(ints);

    printf("test_alocacao: OK\n");
    return 0;
}