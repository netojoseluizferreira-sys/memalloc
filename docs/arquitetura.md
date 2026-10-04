# Arquitetura do memalloc

Documento técnico sobre as decisões de design do alocador.

## Visão geral

O `memalloc` é um alocador de memória simplificado que gerencia uma região
fixa de 4096 bytes fornecida pelo próprio alocador. Não usa `sbrk` nem
`mmap` — trabalha sobre um buffer alocado uma única vez com `malloc`.

O objetivo é didático: entender como o `malloc` funciona por baixo,
incluindo divisão de blocos, liberação e coalescência.

## Modelo de memória

O buffer é dividido em **blocos**. Cada bloco tem dois componentes:

1. **Cabeçalho de metadados** (`struct Bloco`)
2. **Dados** — a região que o usuário usa

Layout:

```
buffer:
┌──────────────────────┬──────────────┬──────────────────────┬──────────────┐
│ Bloco 1 (cabeçalho)  │ dados do 1   │ Bloco 2 (cabeçalho)  │ dados do 2   │
└──────────────────────┴──────────────┴──────────────────────┴──────────────┘
```

Os cabeçalhos formam uma **lista encadeada** que percorre o buffer inteiro.

### Estrutura do cabeçalho

```c
typedef struct Bloco {
    size_t tamanho;       // bytes de dados disponíveis
    int livre;            // 1 = livre, 0 = ocupado
    struct Bloco *prox;   // próximo bloco na lista
} Bloco;
```

- **`tamanho`**: contagem apenas dos bytes de **dados** (não inclui o
  próprio cabeçalho).
- **`livre`**: estado do bloco.
- **`prox`**: próximo bloco. **Não existe `ant`** — o alocador não
  precisa navegar para trás.

## Operações

### `memalloc_iniciar`

1. Aloca o buffer (4096 bytes) com `malloc`.
2. Reinterpreta o início do buffer como o primeiro `Bloco`.
3. O primeiro bloco cobre todo o buffer, exceto o próprio cabeçalho:
   `tamanho = 4096 - sizeof(Bloco)`.
4. Marca como livre, com `prox = NULL`.

**Idempotente:** chamar duas vezes não reinicia. Evita vazar o buffer
anterior.

### `memalloc_alocar`

1. Percorre a lista buscando bloco `livre` com `tamanho >= pedido`.
2. Se não achar, retorna `NULL`.
3. Se a sobra (`tamanho - pedido`) for maior que um cabeçalho,
   **divide** o bloco em dois:
   - O atual fica com o tamanho pedido.
   - Um novo bloco (criado logo depois dos dados) recebe o resto.
4. Marca o bloco atual como ocupado.
5. Retorna ponteiro para **depois do cabeçalho** (onde estão os dados).

**Custo:** O(n) — percorre a lista até achar.

### `memalloc_liberar`

1. Acha o bloco a partir do ponteiro de dados (aritmética inversa).
2. Marca como livre.
3. **Coalescência com o próximo:** se o próximo também está livre, absorve
   (soma cabeçalho + dados) e pula na lista.
4. **Coalescência com o anterior:** se o anterior também está livre,
   absorve o bloco atual nele.

**Ordem importa:** próximo antes de anterior. Se fosse o contrário, ao
absorver o bloco no anterior, perderíamos a referência para o próximo.

**Custo:** O(n) — para achar o anterior, percorre a lista do início.

### `memalloc_estatisticas`

1. Percorre a lista somando bytes de dados livres e ocupados.
2. Conta blocos.
3. Imprime o relatório.

**Nota:** a diferença entre `Total` e `Livre + Ocupado` é o **overhead**
dos cabeçalhos. Se há `N` blocos, `N * sizeof(Bloco)` bytes são consumidos
por metadados.

## Decisões de design

### Por que `unsigned char *` para aritmética de bytes

Ponteiros em C avançam por **tamanho do tipo**. Se você fizer
`(Bloco *)p + N`, avança `N * sizeof(Bloco)` bytes. Isso não é o que
queremos quando somamos "N bytes" arbitrários.

Convertendo para `unsigned char *` antes, cada `+ N` avança exatamente
`N` bytes (porque `sizeof(char) == 1`).

### Por que `tamanho` exclui o cabeçalho

Simplifica o cálculo do usuário: se ele pede `N` bytes, o bloco terá
`tamanho = N`, e ele pode usar `N` bytes exatamente. O cabeçalho é
responsabilidade interna do alocador.

### Por que não há `ant`

O alocador não precisa navegar para trás na lista. A única situação em
que precisa é a coalescência com o anterior — e isso é resolvido
percorrendo a lista do início quando necessário.

Adicionar `ant` a cada bloco aumentaria o custo de memória sem
benefício real para as operações atuais.

### Por que aceitar `still reachable` no Valgrind

Não há função de finalização. O buffer é alocado em `memalloc_iniciar`
e permanece vivo até o programa terminar. Isso é comportamento
**esperado** de um alocador — o próprio `malloc` do sistema faz o mesmo
(o heap global não é liberado ao terminar).

O Valgrind reporta isso como `still reachable`, o que não é leak de
verdade. Só é problema se houver `definitely lost`.

## Limitações conhecidas

- **Buffer fixo em 4096 bytes** — não configurável em tempo de execução.
- **Não há detecção de double-free** — liberar o mesmo ponteiro duas
  vezes gera comportamento indefinido.
- **Não há validação de ponteiro** — passar ponteiro inválido para
  `memalloc_liberar` resulta em corrupção.
- **Coalescência com o anterior é O(n)** — percorre a lista do início.
- **Sem thread safety** — não é seguro usar de múltiplas threads.
- **Não há `realloc`** — realocação exigiria copiar dados.
- **Fragmentação interna:** quando a sobra é menor que um cabeçalho, o
  bloco não é dividido e os bytes sobram "presos" no bloco atual.
