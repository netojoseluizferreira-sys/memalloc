#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>

#define MEMALLOC_TAMANHO_PADRAO 4096

/* Inicializa o alocador com o buffer padrão.
 * Retorna 1 em sucesso, 0 em falha. */
int memalloc_iniciar(void);

/* Aloca 'tamanho' bytes do buffer.
 * Retorna ponteiro para os dados ou NULL se não houver espaço. */
void *memalloc_alocar(size_t tamanho);

/* Libera o bloco previamente alocado por memalloc_alocar. */
void memalloc_liberar(void *ptr);

/* Imprime estatísticas de uso: total, livre, usado, blocos. */
void memalloc_estatisticas(void);

#endif