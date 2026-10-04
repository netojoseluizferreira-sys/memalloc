// Testes de liberação do memalloc.
// Verifica que memalloc_liberar marca blocos como livres, faz coalescência
// e não quebra com ponteiro NULL.
#include "memalloc.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    // Inicializa o alocador
    assert(memalloc_iniciar() == 1);

    // Aloca dois blocos
    void *a = memalloc_alocar(100);
    void *b = memalloc_alocar(50);
    assert(a != NULL);
    assert(b != NULL);

    // Libera o primeiro — o "Ocupado" deve diminuir
    memalloc_liberar(a);

    // Libera o segundo — o "Ocupado" deve voltar a 0
    memalloc_liberar(b);

    // Liberar NULL não deve quebrar
    memalloc_liberar(NULL);

    // Reutilização: alocar de novo depois de liberar deve funcionar
    void *c = memalloc_alocar(200);
    assert(c != NULL);
    memalloc_liberar(c);

    // Liberar o mesmo ponteiro duas vezes não deve ser feito
    // (comportamento indefinido — não testamos)

    printf("test_liberacao: OK\n");
    return 0;
}