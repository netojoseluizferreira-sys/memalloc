# memalloc

Alocador de memória simplificado, implementado do zero em C.

Projeto integrador do **Bloco 0 — Fundição do Ferro** do C Crucible.

## Sobre

Um mini-heap que gerencia uma região de memória fornecida, com controle
manual de blocos livres e ocupados. O objetivo é entender como o `malloc`
funciona por baixo — sem tratá-lo como caixa-preta.

O alocador não usa `sbrk` nem `mmap`. Ele trabalha sobre um buffer estático
que simula o heap, implementando do zero:

- Lista encadeada de blocos de metadados
- Estratégia de alocação e divisão de blocos
- Liberação com marcação
- Estatísticas de uso (total, livre, ocupado)

## Estrutura

memalloc/

├── include/     # interface pública (memalloc.h)

├── src/         # implementação (memalloc.c)

├── tests/       # testes automatizados

├── examples/    # exemplos de uso

└── docs/        # arquitetura e decisões de design


## Compilar

Requer GCC e Make (Linux, macOS ou WSL).

```bash
make            # compila o exemplo
make test       # compila e roda os testes
make debug      # compila com AddressSanitizer + UBSan
make valgrind   # roda o exemplo sob Valgrind
make clean      # remove binários gerados
```

## Conceitos aplicados

- Ponteiros e aritmética de ponteiros
- Structs e `typedef`
- `void *` e casts explícitos
- `malloc` / `free`
- Lista encadeada aplicada a metadados
- Layout de memória e alinhamento
- Gerenciamento manual de recursos

## Documentação

Detalhes de arquitetura, decisões de design e trade-offs ficam em
[`docs/arquitetura.md`](docs/arquitetura.md).

## Contexto

Este projeto faz parte de uma trilha de estudos pessoal em C avançado,
organizada em blocos progressivos. O `memalloc` fecha o primeiro bloco,
consolidando os fundamentos de ponteiros e gerenciamento de memória
antes de avançar para estruturas de dados.

## Licença

MIT — veja [LICENSE](LICENSE).
