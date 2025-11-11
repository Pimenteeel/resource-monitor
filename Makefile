# --- Variáveis do Compilador ---

# O compilador que vamos usar
CC = gcc

# Flags de compilação:
# -Wall -Wextra = Avisos rigorosos (como você pediu)
# -Iinclude      = Diz ao GCC para procurar arquivos .h na pasta "include/"
# -g             = Adiciona símbolos de debug (para usar com gdb)
CFLAGS = -Wall -Wextra -Iinclude -g

# --- Variáveis do Projeto ---

# O nome do seu programa final
TARGET = resource-monitor

# Encontra automaticamente TODOS os arquivos .c dentro da pasta src/
# (Ex: src/main.c, src/cpu_monitor.c, etc.)
SRCS = $(wildcard src/*.c)

# Cria a lista de arquivos de objeto (.o) correspondentes
# (Ex: src/main.o, src/cpu_monitor.o, etc.)
OBJS = $(SRCS:.c=.o)

# Encontra automaticamente TODOS os arquivos .h na pasta include/
HEADERS = $(wildcard include/*.h)	

# --- Regras (Rules) ---

# A regra padrão (o que acontece quando você digita "make")
# "all" depende do seu programa (TARGET)
all: $(TARGET)

# Regra de "Linkagem":
# Como criar o programa final (TARGET)
# Depende de todos os arquivos de objeto (.o)
# Ele junta todos os .o em um único executável
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

# Regra de "Compilação":
# Como criar um arquivo .o a partir de um arquivo .c
# Depende do arquivo .c correspondente E de qualquer arquivo .h
# $< = O arquivo .c (pré-requisito)
# $@ = O arquivo .o (o alvo)
src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra "clean":
# O que acontece quando você digita "make clean"
# Remove todos os arquivos .o e o programa final
clean:
	rm -f $(OBJS) $(TARGET)

# Regra "rebuild":
# O que acontece quando você digita "make re"
# Limpa tudo e compila do zero
re: clean all

# Avisa ao 'make' que 'all', 'clean', e 're' não são nomes de arquivos
.PHONY: all clean re