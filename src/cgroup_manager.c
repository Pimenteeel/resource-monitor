#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
bool write_to_file(const char* filepath, const char* value) {
    FILE* file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "Erro ao abrir %s: %s\n", filepath, strerror(errno));
        return false;
    }
    
    if (fprintf(file, "%s", value) < 0) {
        fprintf(stderr, "Erro ao escrever em %s: %s\n", filepath, strerror(errno));
        fclose(file);
        return false;
    }
    
    fclose(file);
    return true;
}

unsigned long long read_from_file_ull(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir %s: %s\n", filepath, strerror(errno));
        return 0;
    }
    
    unsigned long long value;
    if (fscanf(file, "%llu", &value) != 1) {
        fprintf(stderr, "Erro ao ler de %s\n", filepath);
        fclose(file);
        return 0;
    }
    
    fclose(file);
    return value;
}

long read_from_file_long(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir %s: %s\n", filepath, strerror(errno));
        return -1;
    }
    
    long value;
    if (fscanf(file, "%ld", &value) != 1) {
        fprintf(stderr, "Erro ao ler de %s\n", filepath);
        fclose(file);
        return -1;
    }
    
    fclose(file);
    return value;
}


bool coletar_metricas_cgroup(const char* cgroup_path) {
    printf("COLETANDO METRICAS DO CGROUP: %s\n", cgroup_path);
    
    struct stat st;
    if (stat(cgroup_path, &st) != 0) {
        fprintf(stderr, "ERRO: Cgroup nao existe: %s\n", cgroup_path);
        return false;
    }
    
    printf("----------------------------------------\n");
    printf("METRICA                      | VALOR    \n");
    printf("----------------------------------------\n");
    
    // Ler métricas reais dos arquivos
    char metric_path[MAX_PATH];
    
    // CPU usage (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/cpu.stat", cgroup_path);
    FILE* cpu_file = fopen(metric_path, "r");
    unsigned long long cpu_usage = 0;
    if (cpu_file) {
        char line[256];
        while (fgets(line, sizeof(line), cpu_file)) {
            if (strstr(line, "usage_usec")) {
                sscanf(line, "usage_usec %llu", &cpu_usage);
                cpu_usage *= 1000; // converter para nanosegundos
            }
        }
        fclose(cpu_file);
        printf("Uso de CPU (cpu.stat)       | %llu ns\n", cpu_usage);
    } else {
        printf("Uso de CPU (cpu.stat)       | 0 ns\n");
    }
    
    // Memory usage (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/memory.current", cgroup_path);
    long memory_usage = read_from_file_long(metric_path);
    printf("Uso de Memoria              | %.1f MB\n", (double)memory_usage / (1024 * 1024));
    
    // Memory limit (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/memory.max", cgroup_path);
    long memory_limit = read_from_file_long(metric_path);
    if (memory_limit == 9223372036854775807L) { // valor máximo significa sem limite
        printf("Limite de Memoria           | SEM LIMITE\n");
    } else {
        printf("Limite de Memoria           | %.1f MB\n", (double)memory_limit / (1024 * 1024));
    }
    
    // Memory fail count (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/memory.events", cgroup_path);
    FILE* mem_events = fopen(metric_path, "r");
    long memory_failcnt = 0;
    if (mem_events) {
        char line[256];
        while (fgets(line, sizeof(line), mem_events)) {
            if (strstr(line, "oom_kill")) {
                sscanf(line, "oom_kill %ld", &memory_failcnt);
            }
        }
        fclose(mem_events);
        printf("Falhas de Memoria (OOM)     | %ld\n", memory_failcnt);
    } else {
        printf("Falhas de Memoria (OOM)     | 0\n");
    }
    
    // PIDs current (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/cgroup.procs", cgroup_path);
    FILE* procs_file = fopen(metric_path, "r");
    long pids_current = 0;
    if (procs_file) {
        char line[32];
        while (fgets(line, sizeof(line), procs_file)) {
            pids_current++;
        }
        fclose(procs_file);
        printf("PIDs atuais                 | %ld\n", pids_current);
    } else {
        printf("PIDs atuais                 | 0\n");
    }
    
    printf("----------------------------------------\n");
    
    return true;
}


bool criar_configurar_cgroup(const char* controller, const char* name, double cpu_cores, long memoria_mb) {
    char cgroup_path[MAX_PATH];
    // CGROUPS V2: tudo na mesma pasta
    snprintf(cgroup_path, sizeof(cgroup_path), "/sys/fs/cgroup/%s", name);
    
    printf("CRIANDO E CONFIGURANDO CGROUP:\n");
    printf("Controlador: %s\n", controller);
    printf("Nome: %s\n", name);
    printf("CPU: %.2f cores\n", cpu_cores);
    printf("Memoria: %ld MB\n", memoria_mb);
    
    // Criar diretório do cgroup
    if (mkdir(cgroup_path, 0755) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Erro ao criar cgroup %s: %s\n", cgroup_path, strerror(errno));
            return false;
        }
        printf("Cgroup ja existe: %s\n", cgroup_path);
    } else {
        printf("Cgroup criado: %s\n", cgroup_path);
    }
    
    // Configurar limite de CPU se especificado 
    if (cpu_cores > 0) {
        long quota_us = (long)(cpu_cores * 100000);
        
        char filepath[MAX_PATH * 2];
        snprintf(filepath, sizeof(filepath), "%s/cpu.max", cgroup_path);
        char quota_str[32];
        snprintf(quota_str, sizeof(quota_str), "%ld 100000", quota_us);
        
        if (!write_to_file(filepath, quota_str)) {
            fprintf(stderr, "Erro ao configurar limite de CPU\n");
            return false;
        }
        
        printf("CPU configurado: max=%ldus, period=100000us\n", quota_us);
    }
    
    if (memoria_mb > 0) {
        long memoria_bytes = memoria_mb * 1024 * 1024;
        
        char filepath[MAX_PATH];
        snprintf(filepath, sizeof(filepath), "%s/memory.max", cgroup_path);
        char memory_str[32];
        snprintf(memory_str, sizeof(memory_str), "%ld", memoria_bytes);
        
        if (!write_to_file(filepath, memory_str)) {
            fprintf(stderr, "Erro ao configurar limite de memoria\n");
            return false;
        }
        
        printf("Memoria configurada: %ld bytes\n", memoria_bytes);
    }
    
    printf("Cgroup criado e configurado com sucesso\n");
    return true;
}



bool move_process_to_cgroup(const char* cgroup_path, int pid) {
    char filepath[MAX_PATH];
    
    if (pid <= 0) {
        fprintf(stderr, "PID invalido: %d\n", pid);
        return false;
    }
    
    snprintf(filepath, sizeof(filepath), "%s/cgroup.procs", cgroup_path);
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    
    if (!write_to_file(filepath, pid_str)) {
        fprintf(stderr, "Erro ao mover processo %d para cgroup\n", pid);
        return false;
    }
    
    printf("Processo %d movido para cgroup: %s\n", pid, cgroup_path);
    return true;
}


void gerar_relatorio_utilizacao(const char* cgroup_path) {
    printf("GERANDO RELATORIO DE UTILIZACAO: %s\n", cgroup_path);
    
    FILE* relatorio = fopen("cgroup_report.csv", "w");
    if (!relatorio) {
        perror("Erro ao criar relatorio");
        return;
    }
    
    fprintf(relatorio, "timestamp,metric,value,limit,utilization\n");
    
    time_t now = time(NULL);
    char metric_path[MAX_PATH];
    
    // CPU (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/cpu.stat", cgroup_path);
    FILE* cpu_file = fopen(metric_path, "r");
    unsigned long long cpu_usage = 0;
    if (cpu_file) {
        char line[256];
        while (fgets(line, sizeof(line), cpu_file)) {
            if (strstr(line, "usage_usec")) {
                sscanf(line, "usage_usec %llu", &cpu_usage);
                cpu_usage *= 1000;
            }
        }
        fclose(cpu_file);
    }
    fprintf(relatorio, "%ld,cpu_usage,%llu,0,0\n", now, cpu_usage);
    
    // Memory (cgroups v2)
    snprintf(metric_path, sizeof(metric_path), "%s/memory.current", cgroup_path);
    long memory_usage = read_from_file_long(metric_path);
    snprintf(metric_path, sizeof(metric_path), "%s/memory.max", cgroup_path);
    long memory_limit = read_from_file_long(metric_path);
    double memory_util = (memory_limit > 0 && memory_limit != 9223372036854775807L) ? 
                         (double)memory_usage / memory_limit * 100 : 0;
    fprintf(relatorio, "%ld,memory_usage,%ld,%ld,%.2f\n", now, memory_usage, memory_limit, memory_util);
    
    fclose(relatorio);
    printf("Relatorio gerado: cgroup_report.csv\n");
}

bool empty_and_delete_cgroup(const char* cgroup_path) {
    char procs_path[MAX_PATH];
    snprintf(procs_path, sizeof(procs_path), "%s/cgroup.procs", cgroup_path);
    
    if (!write_to_file(procs_path, "0")) {
        fprintf(stderr, "Erro ao esvaziar cgroup %s\n", cgroup_path);
        return false;
    }
    
    sleep(1);
    
    if (rmdir(cgroup_path) != 0) {
        fprintf(stderr, "Erro ao deletar cgroup %s: %s\n", cgroup_path, strerror(errno));
        return false;
    }
    
    printf("Cgroup deletado: %s\n", cgroup_path);
    return true;
}