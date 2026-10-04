// Exemplo básico do alocador memalloc.
// Demonstra iniciar, alocar, liberar e ver as estatísticas de uso.
#include "memalloc.h"
#include <stdio.h>

int main(void) {
    // Inicializa o alocador
    if (!memalloc_iniciar()) {
        printf("Erro: falha ao iniciar o alocador\n");
        return 1;
    }

    // Aloca dois blocos
    int *a = (int *)memalloc_alocar(10 * sizeof(int));
    int *b = (int *)memalloc_alocar(5 * sizeof(int));

    if (a == NULL || b == NULL) {
        printf("Erro: falha na alocação\n");
        return 1;
    }

    // Escreve dados nos blocos para provar que são usáveis
    for (int i = 0; i < 10; i++) a[i] = i;
    for (int i = 0; i < 5; i++)  b[i] = i * 10;

    printf("a: ");
    for (int i = 0; i < 10; i++) printf("%d ", a[i]);
    printf("\n");

    printf("b: ");
    for (int i = 0; i < 5; i++) printf("%d ", b[i]);
    printf("\n\n");

    // Estatísticas com dois blocos ocupados
    memalloc_estatisticas();

    // Libera os dois blocos
    memalloc_liberar(a);
    memalloc_liberar(b);

    printf("\n");

    // Estatísticas depois de liberar (o buffer deve estar de volta ao estado inicial)
    memalloc_estatisticas();

    return 0;
}