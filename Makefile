# --- Variáveis do Compilador ---

# O compilador que vamos usar
CC = gcc

# Flags de compilação:
# -Wall -Wextra = Avisos rigorosos
# -std=gnu11    = Usa o padrão C11 com extensões GNU (necessário para 'clone()')
# -Iinclude     = Diz ao GCC para procurar arquivos .h na pasta "include/"
# -g            = Adiciona símbolos de debug (para usar com gdb)
CFLAGS = -Wall -Wextra -std=gnu11 -Iinclude -g

# Flags de Linkagem:
# -lrt          = Linka a biblioteca de tempo real (necessário para 'clock_gettime')
LDFLAGS = -lrt

# --- Variáveis do Projeto ---

# O nome do seu programa final
TARGET = resource-monitor

# Encontra automaticamente TODOS os arquivos .c dentro da pasta src/
# (Ex: src/main.c, src/cpu_monitor.c, src/namespace_analyzer.c, etc.)
SRCS = $(wildcard src/*.c)

# Cria a lista de arquivos de objeto (.o) correspondentes
# (Ex: src/main.o, src/cpu_monitor.o, etc.)
OBJS = $(SRCS:.c=.o)

# Encontra automaticamente TODOS os arquivos .h na pasta include/
HEADERS = $(wildcard include/*.h)

# --- Regras (Rules) ---

# A regra padrão (o que acontece quando você digita "make")
all: $(TARGET)

# Regra de "Linkagem":
# Como criar o programa final (TARGET)
# Junta todos os .o em um único executável e aplica as flags de linkagem.
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Regra de "Compilação":
# Como criar um arquivo .o a partir de um arquivo .c
# Depende do arquivo .c correspondente E de qualquer arquivo .h
# $< = O arquivo .c (pré-requisito)
# $@ = O arquivo .o (o alvo)
src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra "clean":
# Remove todos os arquivos .o e o programa final
clean:
	-rm -f $(OBJS) $(TARGET)

# Regra "rebuild":
# Limpa tudo e compila do zero
re: clean all

# Avisa ao 'make' que 'all', 'clean', e 're' não são nomes de arquivos
.PHONY: all clean re

# ============================================================================
# REGRAS PARA TESTES (ACRESCENTADAS - SEM MODIFICAR O EXISTENTE)
# ============================================================================

# Compilar teste CPU-intensive
test_cpu: tests/test_cpu.c
	$(CC) $(CFLAGS) -o test_cpu tests/test_cpu.c -lm -lpthread

# Compilar teste Memory-intensive  
test_memory: tests/test_memory.c
	$(CC) $(CFLAGS) -o test_memory tests/test_memory.c

# Compilar teste I/O-intensive
test_io: tests/test_io.c
	$(CC) $(CFLAGS) -o test_io tests/test_io.c

# Compilar todos os testes
tests: test_cpu test_memory test_io
	@echo "Todos os testes compilados!"

# Limpar testes
clean-tests:
	-rm -f test_cpu test_memory test_io
	-rm -f test_file_*.dat

# Limpar tudo (projeto + testes)
clean-all: clean clean-tests

# Regras phony para os testes
.PHONY: test_cpu test_memory test_io tests clean-tests clean-all