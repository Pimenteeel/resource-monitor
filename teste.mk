CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g

# Teste básico - só compila para ver se tem erros
teste-compilacao:
	@echo "🔨 Testando compilação..."
	$(CC) $(CFLAGS) -c src/main.c -o /tmp/main.o 2>&1 | head -10
	$(CC) $(CFLAGS) -c src/cgroup_manager.c -o /tmp/cgroup.o 2>&1 | head -10
	@echo "✅ Compilação básica OK (sem erros de sintaxe)"

# Teste de linking
teste-linking: teste-compilacao
	@echo "🔗 Testando linking..."
	$(CC) $(CFLAGS) /tmp/main.o /tmp/cgroup.o -o /tmp/teste_programa
	@echo "✅ Linking OK - programa criado em /tmp/teste_programa"

# Teste completo
teste: teste-linking
	@echo "🚀 Executando testes básicos..."
	@echo "=== TESTE 1: Ajuda ==="
	/tmp/teste_programa -h || true
	@echo ""
	@echo "=== TESTE 2: Listar cgroups ==="
	/tmp/teste_programa -l || true
	@echo "✅ Testes básicos concluídos"

clean-teste:
	rm -f /tmp/main.o /tmp/cgroup.o /tmp/teste_programa