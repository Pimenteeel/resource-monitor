#include "cgroup.h"
#include <math.h>
#include <signal.h>

// Variável global para histórico
static CgroupHistory *global_history = NULL;

// ============================================================================
// FUNÇÕES DE GERENCIAMENTO DO KERNEL (OBRIGATÓRIAS)
// ============================================================================

// Função auxiliar para escrever em arquivos do cgroup
static int write_cgroup_file(const char *cgroup_path, const char *filename, const char *value) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", cgroup_path, filename);
    
    FILE *file = fopen(filepath, "w");
    if (!file) {
        perror("Erro ao abrir arquivo do cgroup para escrita");
        return -1;
    }
    
    if (fprintf(file, "%s", value) < 0) {
        perror("Erro ao escrever no arquivo do cgroup");
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return 0;
}

// Função auxiliar para ler de arquivos do cgroup
static int read_cgroup_file(const char *cgroup_path, const char *filename, char *buffer, size_t buffer_size) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", cgroup_path, filename);
    
    FILE *file = fopen(filepath, "r");
    if (!file) {
        return -1;
    }
    
    if (!fgets(buffer, buffer_size, file)) {
        fclose(file);
        return -1;
    }
    
    buffer[strcspn(buffer, "\n")] = 0;
    fclose(file);
    return 0;
}

// Função auxiliar para construir caminho do cgroup
static void build_cgroup_path(const char *cgroup_name, char *path_buffer) {
    snprintf(path_buffer, 512, "%s/%s", CGROUP_BASE_PATH, cgroup_name);
}

int cgroup_create(const char *cgroup_name) {
    char cgroup_path[512];
    build_cgroup_path(cgroup_name, cgroup_path);
    
    if (mkdir(cgroup_path, 0755) != 0) {
        if (errno == EEXIST) {
            printf("Cgroup '%s' ja existe\n", cgroup_name);
            return 0;
        }
        perror("Erro ao criar cgroup");
        return -1;
    }
    
    printf("Cgroup '%s' criado com sucesso\n", cgroup_name);
    return 0;
}

int cgroup_delete(const char *cgroup_name) {
    char cgroup_path[512];
    build_cgroup_path(cgroup_name, cgroup_path);
    
    // Primeiro, remover todos os processos do cgroup
    write_cgroup_file(cgroup_path, "cgroup.procs", "0");
    
    if (rmdir(cgroup_path) != 0) {
        perror("Erro ao deletar cgroup");
        return -1;
    }
    
    printf("Cgroup '%s' deletado com sucesso\n", cgroup_name);
    return 0;
}

int cgroup_add_process(const char *cgroup_name, int pid) {
    char cgroup_path[512];
    build_cgroup_path(cgroup_name, cgroup_path);
    
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    
    if (write_cgroup_file(cgroup_path, "cgroup.procs", pid_str) != 0) {
        fprintf(stderr, "Erro ao adicionar processo %d ao cgroup '%s'\n", pid, cgroup_name);
        return -1;
    }
    
    printf("Processo %d adicionado ao cgroup '%s'\n", pid, cgroup_name);
    return 0;
}

int cgroup_set_limits(const char *cgroup_name, const CgroupLimits *limits) {
    char cgroup_path[512];
    build_cgroup_path(cgroup_name, cgroup_path);
    char value[64];
    int success_count = 0;
    
    // Configurar limite de CPU
    if (limits->cpu_limit > 0) {
        long period = 100000; // 100ms em microssegundos
        long quota = (long)(limits->cpu_limit * period);
        
        snprintf(value, sizeof(value), "%ld", period);
        if (write_cgroup_file(cgroup_path, "cpu.cfs_period_us", value) == 0) {
            snprintf(value, sizeof(value), "%ld", quota);
            if (write_cgroup_file(cgroup_path, "cpu.cfs_quota_us", value) == 0) {
                success_count++;
                printf("Limite de CPU: %.2f cores\n", limits->cpu_limit);
            }
        }
    }
    
    // Configurar limite de memoria
    if (limits->memory_limit > 0) {
        snprintf(value, sizeof(value), "%ld", limits->memory_limit);
        if (write_cgroup_file(cgroup_path, "memory.limit_in_bytes", value) == 0) {
            success_count++;
            printf("Limite de Memoria: %.2f MB\n", limits->memory_limit / (1024.0 * 1024.0));
        }
    }
    
    // Configurar limite de PIDs
    if (limits->pids_limit > 0) {
        snprintf(value, sizeof(value), "%ld", limits->pids_limit);
        if (write_cgroup_file(cgroup_path, "pids.max", value) == 0) {
            success_count++;
            printf("Limite de PIDs: %ld\n", limits->pids_limit);
        }
    }
    
    printf("%d limites configurados para cgroup '%s'\n", success_count, cgroup_name);
    return success_count;
}

int cgroup_get_metrics(const char *cgroup_name, CgroupMetrics *metrics) {
    char cgroup_path[512];
    build_cgroup_path(cgroup_name, cgroup_path);
    char buffer[128];
    
    // Inicializar estrutura
    memset(metrics, 0, sizeof(CgroupMetrics));
    metrics->timestamp = time(NULL);
    
    // Ler metricas de CPU
    if (read_cgroup_file(cgroup_path, "cpuacct.usage", buffer, sizeof(buffer)) == 0) {
        metrics->cpu_usage = atol(buffer);
    }
    
    if (read_cgroup_file(cgroup_path, "cpuacct.usage_user", buffer, sizeof(buffer)) == 0) {
        metrics->cpu_usage_user = atol(buffer);
    }
    
    if (read_cgroup_file(cgroup_path, "cpuacct.usage_system", buffer, sizeof(buffer)) == 0) {
        metrics->cpu_usage_system = atol(buffer);
    }
    
    // Ler metricas de memoria
    if (read_cgroup_file(cgroup_path, "memory.usage_in_bytes", buffer, sizeof(buffer)) == 0) {
        metrics->memory_usage = atol(buffer);
    }
    
    if (read_cgroup_file(cgroup_path, "memory.limit_in_bytes", buffer, sizeof(buffer)) == 0) {
        metrics->memory_limit = atol(buffer);
    }
    
    if (read_cgroup_file(cgroup_path, "memory.failcnt", buffer, sizeof(buffer)) == 0) {
        metrics->memory_failcnt = atol(buffer);
    }
    
    // Ler metricas de PIDs
    if (read_cgroup_file(cgroup_path, "pids.current", buffer, sizeof(buffer)) == 0) {
        metrics->pids_current = atol(buffer);
    }
    
    if (read_cgroup_file(cgroup_path, "pids.max", buffer, sizeof(buffer)) == 0) {
        if (strcmp(buffer, "max") == 0) {
            metrics->pids_limit = -1; // Ilimitado
        } else {
            metrics->pids_limit = atol(buffer);
        }
    }
    
    return 0;
}

int cgroup_list_all(char ***cgroups_list, int *count) {
    DIR *dir;
    struct dirent *entry;
    char **list = NULL;
    int c = 0;
    
    dir = opendir(CGROUP_BASE_PATH);
    if (!dir) {
        perror("Erro ao abrir diretorio de cgroups");
        return -1;
    }
    
    // Contar cgroups
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            c++;
        }
    }
    
    rewinddir(dir);
    
    // Alocar memoria para a lista
    list = malloc(c * sizeof(char*));
    if (!list) {
        closedir(dir);
        return -1;
    }
    
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < c) {
        if (entry->d_type == DT_DIR && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            
            list[i] = strdup(entry->d_name);
            if (!list[i]) {
                // Liberar memoria em caso de erro
                for (int j = 0; j < i; j++) {
                    free(list[j]);
                }
                free(list);
                closedir(dir);
                return -1;
            }
            i++;
        }
    }
    
    closedir(dir);
    *cgroups_list = list;
    *count = c;
    return 0;
}

void cgroup_free_list(char **cgroups_list, int count) {
    for (int i = 0; i < count; i++) {
        free(cgroups_list[i]);
    }
    free(cgroups_list);
}

void cgroup_print_metrics(const CgroupMetrics *metrics) {
    printf("\n=== METRICAS DO CGROUP ===\n");
    printf("Timestamp: %s", ctime(&metrics->timestamp));
    printf("CPU Usage: %.2f ms\n", metrics->cpu_usage / 1000000.0);
    printf("CPU User: %.2f ms\n", metrics->cpu_usage_user / 1000000.0);
    printf("CPU System: %.2f ms\n", metrics->cpu_usage_system / 1000000.0);
    printf("Memory Usage: %.2f MB", metrics->memory_usage / (1024.0 * 1024.0));
    
    if (metrics->memory_limit > 0) {
        printf(" / %.2f MB (%.1f%%)\n", 
               metrics->memory_limit / (1024.0 * 1024.0),
               (metrics->memory_usage * 100.0) / metrics->memory_limit);
    } else {
        printf(" / unlimited\n");
    }
    
    printf("Memory Failures: %ld\n", metrics->memory_failcnt);
    printf("PIDs: %ld", metrics->pids_current);
    
    if (metrics->pids_limit > 0) {
        printf(" / %ld (%.1f%%)\n", metrics->pids_limit,
               (metrics->pids_current * 100.0) / metrics->pids_limit);
    } else {
        printf(" / unlimited\n");
    }
}

// ============================================================================
// FUNÇÕES DE RELATÓRIOS E VISUALIZAÇÃO (OPCIONAIS - PONTOS EXTRAS)
// ============================================================================

CgroupHistory* cgroup_history_create(int initial_capacity) {
    CgroupHistory *history = malloc(sizeof(CgroupHistory));
    if (!history) return NULL;
    
    history->metrics = malloc(initial_capacity * sizeof(CgroupMetrics));
    if (!history->metrics) {
        free(history);
        return NULL;
    }
    
    history->count = 0;
    history->capacity = initial_capacity;
    return history;
}

void cgroup_history_add(CgroupHistory *history, const CgroupMetrics *metrics) {
    if (history->count >= history->capacity) {
        int new_capacity = history->capacity * 2;
        CgroupMetrics *new_metrics = realloc(history->metrics, new_capacity * sizeof(CgroupMetrics));
        if (!new_metrics) return;
        
        history->metrics = new_metrics;
        history->capacity = new_capacity;
    }
    
    history->metrics[history->count] = *metrics;
    history->count++;
}

void cgroup_history_free(CgroupHistory *history) {
    if (history) {
        free(history->metrics);
        free(history);
    }
}

int cgroup_start_monitoring(const char *cgroup_name, int interval_sec, int duration_sec) {
    if (!global_history) {
        global_history = cgroup_history_create(100);
        if (!global_history) {
            return -1;
        }
    }
    
    printf("\nINICIANDO MONITORAMENTO DO CGROUP '%s'\n", cgroup_name);
    printf("Duracao: %d segundos | Intervalo: %d segundos\n", duration_sec, interval_sec);
    
    time_t start_time = time(NULL);
    int samples = 0;
    
    while (time(NULL) - start_time < duration_sec) {
        CgroupMetrics metrics;
        if (cgroup_get_metrics(cgroup_name, &metrics) == 0) {
            metrics.timestamp = time(NULL);
            cgroup_history_add(global_history, &metrics);
            samples++;
            
            printf("Amostra %d - CPU: %.2fms, Mem: %.2fMB, PIDs: %ld\n", 
                   samples, 
                   metrics.cpu_usage / 1000000.0, 
                   metrics.memory_usage / (1024.0 * 1024.0),
                   metrics.pids_current);
        }
        
        sleep(interval_sec);
    }
    
    printf("Monitoramento concluido. %d amostras coletadas.\n", samples);
    return samples;
}

void cgroup_generate_csv_report(const char *cgroup_name, const char *filename) {
    FILE *csv = fopen(filename, "w");
    if (!csv) {
        perror("Erro ao criar arquivo CSV");
        return;
    }
    
    // Cabecalho otimizado para Python
    fprintf(csv, "timestamp,datetime,cpu_usage_ms,memory_usage_mb,memory_limit_mb,pids_current,utilization_percent\n");
    
    if (global_history && global_history->count > 0) {
        for (int i = 0; i < global_history->count; i++) {
            CgroupMetrics *m = &global_history->metrics[i];
            double utilization = (m->memory_limit > 0) ? 
                (m->memory_usage * 100.0) / m->memory_limit : 0.0;
            
            // Converter timestamp para datetime legivel
            char datetime[64];
            struct tm *timeinfo = localtime(&m->timestamp);
            strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            fprintf(csv, "%ld,%s,%.2f,%.2f,%.2f,%ld,%.1f\n",
                    m->timestamp, datetime,
                    m->cpu_usage / 1000000.0,
                    m->memory_usage / (1024.0 * 1024.0),
                    m->memory_limit / (1024.0 * 1024.0),
                    m->pids_current,
                    utilization);
        }
    } else {
        // Relatorio unico se nao houver historico
        CgroupMetrics metrics;
        if (cgroup_get_metrics(cgroup_name, &metrics) == 0) {
            char datetime[64];
            struct tm *timeinfo = localtime(&metrics.timestamp);
            strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            double utilization = (metrics.memory_limit > 0) ? 
                (metrics.memory_usage * 100.0) / metrics.memory_limit : 0.0;
            
            fprintf(csv, "%ld,%s,%.2f,%.2f,%.2f,%ld,%.1f\n",
                    metrics.timestamp, datetime,
                    metrics.cpu_usage / 1000000.0,
                    metrics.memory_usage / (1024.0 * 1024.0),
                    metrics.memory_limit / (1024.0 * 1024.0),
                    metrics.pids_current,
                    utilization);
        }
    }
    
    fclose(csv);
    printf("Relatorio CSV gerado: %s\n", filename);
}

void generate_python_visualization(const char *csv_filename) {
    char py_filename[256];
    snprintf(py_filename, sizeof(py_filename), "%s_visualization.py", csv_filename);
    
    FILE *py = fopen(py_filename, "w");
    if (!py) {
        perror("Erro ao criar script Python");
        return;
    }
    
    fprintf(py, "#!/usr/bin/env python3\n");
    fprintf(py, "\"\"\"\n");
    fprintf(py, "VISUALIZACAO DE METRICAS CGROUP - Gerado automaticamente\n");
    fprintf(py, "Execute: python3 %s\n", py_filename);
    fprintf(py, "\"\"\"\n\n");
    fprintf(py, "import pandas as pd\n");
    fprintf(py, "import matplotlib.pyplot as plt\n");
    fprintf(py, "import sys\n");
    fprintf(py, "import os\n\n");
    fprintf(py, "# Configurar estilo dos graficos\n");
    fprintf(py, "plt.style.use('seaborn-v0_8')\n\n");
    fprintf(py, "def load_data():\n");
    fprintf(py, "    \"\"\"Carrega dados do CSV\"\"\"\n");
    fprintf(py, "    try:\n");
    fprintf(py, "        df = pd.read_csv('%s')\n", csv_filename);
    fprintf(py, "        df['datetime'] = pd.to_datetime(df['datetime'])\n");
    fprintf(py, "        return df\n");
    fprintf(py, "    except Exception as e:\n");
    fprintf(py, "        print(f\"Erro ao carregar dados: {e}\")\n");
    fprintf(py, "        return None\n\n");
    fprintf(py, "def create_dashboard(df):\n");
    fprintf(py, "    \"\"\"Cria dashboard completo com graficos\"\"\"\n");
    fprintf(py, "    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))\n");
    fprintf(py, "    fig.suptitle('Dashboard de Metricas CGroup', fontsize=16, fontweight='bold')\n");
    fprintf(py, "    \n");
    fprintf(py, "    # Grafico 1: Uso de CPU\n");
    fprintf(py, "    ax1.plot(df['datetime'], df['cpu_usage_ms'], 'b-', linewidth=2, marker='o', markersize=3)\n");
    fprintf(py, "    ax1.set_title('Uso de CPU', fontweight='bold')\n");
    fprintf(py, "    ax1.set_ylabel('CPU (ms)')\n");
    fprintf(py, "    ax1.grid(True, alpha=0.3)\n");
    fprintf(py, "    ax1.tick_params(axis='x', rotation=45)\n");
    fprintf(py, "    \n");
    fprintf(py, "    # Grafico 2: Uso de Memoria\n");
    fprintf(py, "    ax2.plot(df['datetime'], df['memory_usage_mb'], 'r-', linewidth=2, label='Uso')\n");
    fprintf(py, "    if df['memory_limit_mb'].iloc[0] > 0:\n");
    fprintf(py, "        ax2.axhline(y=df['memory_limit_mb'].iloc[0], color='red', linestyle='--', alpha=0.7, label='Limite')\n");
    fprintf(py, "    ax2.set_title('Uso de Memoria', fontweight='bold')\n");
    fprintf(py, "    ax2.set_ylabel('Memoria (MB)')\n");
    fprintf(py, "    ax2.legend()\n");
    fprintf(py, "    ax2.grid(True, alpha=0.3)\n");
    fprintf(py, "    ax2.tick_params(axis='x', rotation=45)\n");
    fprintf(py, "    \n");
    fprintf(py, "    # Grafico 3: Utilizacao\n");
    fprintf(py, "    if 'utilization_percent' in df.columns:\n");
    fprintf(py, "        ax3.plot(df['datetime'], df['utilization_percent'], 'g-', linewidth=2)\n");
    fprintf(py, "        ax3.axhline(y=80, color='orange', linestyle='--', alpha=0.7, label='Alerta 80%%')\n");
    fprintf(py, "        ax3.axhline(y=90, color='red', linestyle='--', alpha=0.7, label='Critico 90%%')\n");
    fprintf(py, "        ax3.set_title('Utilizacao de Memoria', fontweight='bold')\n");
    fprintf(py, "        ax3.set_ylabel('Utilizacao (%%%)')\n");
    fprintf(py, "        ax3.legend()\n");
    fprintf(py, "        ax3.grid(True, alpha=0.3)\n");
    fprintf(py, "        ax3.tick_params(axis='x', rotation=45)\n");
    fprintf(py, "    \n");
    fprintf(py, "    # Grafico 4: PIDs\n");
    fprintf(py, "    ax4.plot(df['datetime'], df['pids_current'], 'purple', linewidth=2, marker='s', markersize=3)\n");
    fprintf(py, "    ax4.set_title('Numero de PIDs', fontweight='bold')\n");
    fprintf(py, "    ax4.set_ylabel('PIDs')\n");
    fprintf(py, "    ax4.grid(True, alpha=0.3)\n");
    fprintf(py, "    ax4.tick_params(axis='x', rotation=45)\n");
    fprintf(py, "    \n");
    fprintf(py, "    plt.tight_layout()\n");
    fprintf(py, "    plt.savefig('%s_dashboard.png', dpi=300, bbox_inches='tight')\n", csv_filename);
    fprintf(py, "    print(f\"Dashboard salvo: %s_dashboard.png\")\n", csv_filename);
    fprintf(py, "\n");
    fprintf(py, "def show_statistics(df):\n");
    fprintf(py, "    \"\"\"Mostra estatisticas dos dados\"\"\"\n");
    fprintf(py, "    print(\"\\n\" + \"=\"*50)\n");
    fprintf(py, "    print(\"ESTATISTICAS DAS METRICAS\")\n");
    fprintf(py, "    print(\"=\"*50)\n");
    fprintf(py, "    \n");
    fprintf(py, "    print(f\"Total de amostras: {len(df)}\")\n");
    fprintf(py, "    print(f\"Periodo: {df['datetime'].iloc[0]} ate {df['datetime'].iloc[-1]}\")\n");
    fprintf(py, "    \n");
    fprintf(py, "    if 'cpu_usage_ms' in df.columns:\n");
    fprintf(py, "        print(f\"\\nCPU:\")\n");
    fprintf(py, "        print(f\"   Media: {df['cpu_usage_ms'].mean():.2f} ms\")\n");
    fprintf(py, "        print(f\"   Maximo: {df['cpu_usage_ms'].max():.2f} ms\")\n");
    fprintf(py, "    \n");
    fprintf(py, "    if 'memory_usage_mb' in df.columns:\n");
    fprintf(py, "        print(f\"\\nMemoria:\")\n");
    fprintf(py, "        print(f\"   Media: {df['memory_usage_mb'].mean():.2f} MB\")\n");
    fprintf(py, "        print(f\"   Maximo: {df['memory_usage_mb'].max():.2f} MB\")\n");
    fprintf(py, "    \n");
    fprintf(py, "    if 'utilization_percent' in df.columns:\n");
    fprintf(py, "        max_util = df['utilization_percent'].max()\n");
    fprintf(py, "        print(f\"\\nUtilizacao Maxima: {max_util:.1f}%%\")\n");
    fprintf(py, "        if max_util > 90:\n");
    fprintf(py, "            print(\"   ALERTA CRITICO: Utilizacao acima de 90%%\")\n");
    fprintf(py, "        elif max_util > 80:\n");
    fprintf(py, "            print(\"   ALERTA: Utilizacao acima de 80%%\")\n");
    fprintf(py, "        else:\n");
    fprintf(py, "            print(\"   Utilizacao dentro dos limites seguros\")\n");
    fprintf(py, "\n");
    fprintf(py, "def main():\n");
    fprintf(py, "    print(\"Iniciando visualizacao de metricas CGroup...\")\n");
    fprintf(py, "    \n");
    fprintf(py, "    df = load_data()\n");
    fprintf(py, "    if df is None:\n");
    fprintf(py, "        sys.exit(1)\n");
    fprintf(py, "    \n");
    fprintf(py, "    print(f\"Dados carregados: {len(df)} amostras\")\n");
    fprintf(py, "    \n");
    fprintf(py, "    # Criar dashboard\n");
    fprintf(py, "    create_dashboard(df)\n");
    fprintf(py, "    \n");
    fprintf(py, "    # Mostrar estatisticas\n");
    fprintf(py, "    show_statistics(df)\n");
    fprintf(py, "    \n");
    fprintf(py, "    print(\"\\nVisualizacao concluida!\")\n");
    fprintf(py, "    print(\"Arquivos gerados:\")\n");
    fprintf(py, "    print(f\"   - {csv_filename} (dados)\")\n");
    fprintf(py, "    print(f\"   - %s_dashboard.png (graficos)\" % '%s')\n", csv_filename);
    fprintf(py, "\n");
    fprintf(py, "if __name__ == \"__main__\":\n");
    fprintf(py, "    main()\n");
    
    fclose(py);
    
    // Tornar executavel
    char command[512];
    snprintf(command, sizeof(command), "chmod +x %s", py_filename);
    system(command);
    
    printf("Script Python gerado: %s\n", py_filename);
    printf("Execute: python3 %s\n", py_filename);
}

// Funcao principal para gerar relatorio completo
void cgroup_generate_comprehensive_report(const char *cgroup_name) {
    printf("\nGERANDO RELATORIO COMPLETO PARA '%s'\n", cgroup_name);
    
    char csv_filename[256];
    snprintf(csv_filename, sizeof(csv_filename), "cgroup_%s_report.csv", cgroup_name);
    
    // Gerar CSV
    cgroup_generate_csv_report(cgroup_name, csv_filename);
    
    // Gerar visualizacao Python
    generate_python_visualization(csv_filename);
    
    printf("\nRELATORIO COMPLETO GERADO!\n");
    printf("Arquivos criados:\n");
    printf("   - %s (dados em CSV)\n", csv_filename);
    printf("   - %s_visualization.py (script Python)\n", csv_filename);
    printf("   - %s_dashboard.png (graficos - apos executar o script)\n", csv_filename);
    printf("\nPara visualizar os graficos, execute:\n");
    printf("   python3 %s_visualization.py\n", csv_filename);
}