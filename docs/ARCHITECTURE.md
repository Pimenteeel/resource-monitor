# Architecture — Resource Monitor Project

Este documento descreve a arquitetura completa do **Resource Monitor**, detalhando cada módulo do sistema, seu papel, sua interação com os demais componentes e as decisões de projeto envolvidas.  
O objetivo é apresentar uma visão clara, modular e profissional da solução.

---

# 1. Visão Geral da Arquitetura

O projeto é composto por quatro grandes subsistemas:

1. **Monitoramento de processos (C / Linux /proc)**  
   Responsável por coletar métricas de CPU, memória, IO, rede, cgroups e namespaces.

2. **Persistência e serialização**  
   Salva as métricas em um arquivo CSV estruturado.

3. **Visualização Web (Flask + HTML)**  
   Exibe as métricas coletadas em gráficos atualizados em tempo real.

4. **Ferramentas auxiliares e experimentos**  
   Comparação com ferramentas nativas (`ps`, `top`, `free`) + geradores de carga (CPU/MEM/IO).

Cada módulo é isolado em um arquivo independente, mantendo o código limpo, simples e bem organizado.

---

# 2. Estrutura Geral do Projeto

resource-monitor/
│
├── cpu_monitor.c
├── memory_monitor.c
├── io_monitor.c
├── rede_monitor.c
├── namespace_analyzer.c
├── cgroup_manager.c
├── main.c
│
├── monitor.h
├── namespace.h
├── cgroup.h
│
├── visualize.py
├── templates/
│ └── dashboard.html
│
├── scripts/
│ └── compare_tools.sh
│
└── tests/
├── test_cpu.c
├── test_memory.c
└── test_io.c


# 3. Subsistema 1 — Coleta de Métricas (C)

Todo monitoramento acontece diretamente via arquivos do kernel expostos em:

/proc/<pid>/
/proc/stat
/proc/net/dev
/proc/<pid>/io


Esse subsistema consiste em quatro monitores especializados:

---

## 3.1 cpu_monitor.c — Métricas de CPU
**Entradas:** `/proc/<pid>/stat` e `/proc/stat`  
**Saídas:** ticks totais, utime, stime, context-switches

### Funcionalidades:
- Calcula uso de CPU em percentual
- Mede context-switches voluntários e involuntários
- Lê clock ticks do sistema para normalização

### Decisão de projeto:
O Linux expõe valores acumulados, então o uso percentual é calculado pela diferença entre duas medições:

cpu_percent = (Δ(utime+stime) / Δ(total_ticks_sistema)) * 100


---

## 3.2 memory_monitor.c — Métricas de Memória
**Entradas:** `/proc/<pid>/status`, `/proc/<pid>/statm`  
**Saídas:** RSS, VSZ, Swap, page faults

### Funcionalidades:
- Converte RSS de páginas para bytes
- Coleta swap utilizado
- Soma page faults maiores e menores

### Observação:
Essa abordagem reflete valores reais do kernel, não estimativas externas.

---

## 3.3 io_monitor.c — IO de disco
**Entrada:** `/proc/<pid>/io`  
**Saída:** taxa de leitura/escrita em MB/s

### Funcionalidades:
- Lê `read_bytes` e `write_bytes`
- Calcula throughput a partir de diferenças temporais

---

## 3.4 rede_monitor.c — Rede
**Entrada:** `/proc/net/dev`  
**Saída:** bytes recebidos/enviados por segundo

### Consideração importante:
O kernel não expõe facilmente o uso de rede por processo.  
Portanto, **o monitor registra o IO de rede do sistema**, o que é documentado como limitação.

---

# 4. Subsistema 2 — Serialização (CSV)

Após coletar todas as métricas, o `main.c` agrega tudo e grava no arquivo:

Resource_Monitor.csv

Formato: imestamp,pid,cpu_percent,rss_mb,vsz_mb,swap_mb,page_faults,io_read_rate,io_write_rate,net_rx,net_tx


O timestamp é incremental para facilitar o gráfico.

---

# 5. Subsistema 3 — Namespaces e Cgroups

---

## 5.1 namespace_analyzer.c
Analisa isolamento do processo, lendo os links simbólicos:

/proc/<pid>/ns/


Campos coletados:
- cgroup
- ipc
- mnt
- net
- pid
- time
- user
- uts

### Utilidade:
Permite demonstrar isolamento entre processos (conceito de containers).

---

## 5.2 cgroup_manager.c
Implementa operações fundamentais:

- Criar cgroup
- Configurar limites de CPU, memória ou IO
- Mover processo para o cgroup
- Ler consumo de cgroup
- Remover cgroup

### Justificativa:
O trabalho exige abordar “Recursos e Containers”, então incluir cgroups é obrigatório para complementar namespaces.

---

# 6. Subsistema 4 — Loop Principal (main.c)

O `main.c` orquestra o monitor:

1. Lê argumentos (`-r`, `-n`, etc.)
2. Inicia variáveis de estado
3. Entra em um loop:
   - coleta todas as métricas
   - calcula diferenças para métricas temporais
   - grava a linha no CSV
   - espera `INTERVALO` segundos

### Funcionalidade adicional:
- Modo de análise de namespaces:  
  ```./resource-monitor -n <PID>```

---

# 7. Subsistema 5 — Dashboard Web (Flask + HTML)

---

## 7.1 visualize.py — Backend
Responsável por:

- Servir o HTML do dashboard
- Ler o CSV usando pandas
- Enviar JSON para o frontend

Rotas principais:

| Rota | Função |
|------|--------|
| `/` | retorna o dashboard HTML |
| `/api/dados` | envia últimas 100 entradas do CSV |

---

## 7.2 dashboard.html — Frontend

Exibe 4 gráficos:

1. CPU (%)
2. Memória (RSS)
3. IO (read/write)
4. Rede (rx/tx)

Características:

- Atualização automática a cada 5 segundos
- Layout responsivo
- Charts independentes

---

# 8. Subsistema 6 — Ferramentas de Teste (experimentos)

Os experimentos são parte obrigatória do trabalho.

### test_cpu.c  
Gera carga intensa com operações matemáticas pesadas.

### test_memory.c  
Aloca ~1 GB e escreve nos blocos para consumo real de RAM.

### test_io.c  
Cria arquivos grandes, escreve e lê — ideal para testar IO real.

Esses programas são usados para validar:
- gráficos
- CSV
- comportamento do monitor

---

# 9. Subsistema 7 — Comparação com utilitários nativos

Arquivo: `compare_tools.sh`

Funções:

- Executa `resource-monitor -r` por 10 segundos
- Compara com:
  - `ps`
  - `top`
  - `free`

Ideal para a parte do relatório que pede análise comparativa.

---

# 10. Fluxo de Execução Completo

+------------------+
| Usuário escolhe |
| um PID |
+------------------+
|
v
+------------------+ +------------------+
| resource-monitor| ----> | CSV incremental |
+------------------+ +------------------+
|
v
+------------------+
| visualize.py |
+------------------+
|
v
+------------------+
| dashboard.html |
| (Chart.js) |
+------------------+


---

# 11. Limitações e Decisões de Projeto

- Métricas de rede são coletadas para o **sistema inteiro**, não para o processo (limitação do /proc).
- Em WSL, cgroups são limitados — podem ser demonstrados conceitualmente.
- O monitor usa um intervalo fixo entre coletas (definido no main).
- Valores muito grandes de IO podem ser impactados pelo cache do SO.

---

# 12. Conclusão

A arquitetura do **Resource Monitor** é modular, robusta e didática.  
Ela foi planejada para demonstrar:

- funcionamento interno do Linux via `/proc`
- uso de namespaces e cgroups
- coleta contínua de métricas reais
- visualização moderna e clara
- experimentos reproduzíveis
- comparação com ferramentas padrão do sistema

Serve como uma implementação completa de monitoramento de recursos aplicada ao contexto de containers e isolamento em sistemas operacionais.

