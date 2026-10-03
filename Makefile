# Compilador e flags de guerra
CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror -pedantic
CFLAGS += -Wshadow -Wconversion -Wsign-conversion -Wformat=2
CFLAGS += -Warray-bounds -Wmissing-prototypes -Wnull-dereference
CFLAGS += -Wstrict-prototypes -Wold-style-definition -Wcast-qual
CFLAGS += -Wwrite-strings -Wstrict-aliasing=3 -fno-common
CFLAGS += -Iinclude -g

# Arquivos do projeto
SRC       = src/memalloc.c
EXEMPLO   = examples/exemplo_basico.c
TESTES    = tests/test_alocacao.c tests/test_liberacao.c tests/test_estatisticas.c

# Binários gerados
BIN_EXEMPLO = exemplo
BIN_TESTES  = $(TESTES:.c=)

# Sanitizers (usar com: make debug)
DEBUG_FLAGS = -g -fsanitize=address,undefined

.PHONY: all exemplo test valgrind debug clean

# Alvo padrão: compila o exemplo
all: exemplo

# Compila o exemplo de uso
exemplo: $(SRC) $(EXEMPLO)
	@echo "[BUILD] Exemplo"
	@$(CC) $(CFLAGS) $^ -o $(BIN_EXEMPLO)
	@echo "[OK]    ./exemplo"

# Compila e roda todos os testes
test: $(SRC) $(TESTES)
	@echo "[TEST] Rodando testes..."
	@for t in $(TESTES); do \
		bin=$${t%.c}; \
		$(CC) $(CFLAGS) $(SRC) $$t -o $$bin || exit 1; \
		./$$bin || exit 1; \
	done
	@echo "[OK] Todos os testes passaram"

# Compila com sanitizers
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean exemplo
	@echo "[DEBUG] Compilado com AddressSanitizer e UBSan"

# Roda o exemplo com Valgrind (requer Valgrind instalado)
valgrind: exemplo
	@echo "[VALGRIND] Rodando exemplo"
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./exemplo

# Limpa binários
clean:
	@echo "[CLEAN] Removendo binários"
	@rm -f $(BIN_EXEMPLO)
	@rm -f $(BIN_TESTES)
	@rm -f *.o