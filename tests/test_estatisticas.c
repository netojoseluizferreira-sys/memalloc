// Testes de estatísticas do memalloc.
// Verifica que memalloc_estatisticas não quebra em estados diferentes.
// Testa: lista vazia, lista com blocos ocupados, lista após liberação.
#include "memalloc.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    // Inicializa o alocador
    assert(memalloc_iniciar() == 1);

    // Estado 1: só o bloco inicial (tudo livre)
    printf("--- Estado inicial ---\n");
    memalloc_estatisticas();

    // Estado 2: dois blocos ocupados + um livre (após divisões)
    void *a = memalloc_alocar(100);
    void *b = memalloc_alocar(50);
    assert(a != NULL);
    assert(b != NULL);

    printf("\n--- Com 2 blocos ocupados ---\n");
    memalloc_estatisticas();

    // Estado 3: após liberar tudo (coalescido em 1 bloco)
    memalloc_liberar(a);
    memalloc_liberar(b);

    printf("\n--- Apos liberacao total ---\n");
    memalloc_estatisticas();

    // Estado 4: chamar estatísticas várias vezes seguidas não deve quebrar
    memalloc_estatisticas();
    memalloc_estatisticas();

    printf("\ntest_estatisticas: OK\n");
    return 0;
}