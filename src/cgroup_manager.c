#include "cgroup.h"
#include <stdio.h>

// Implementações MÍNIMAS só para compilar e testar
int cgroup_create(const char *cgroup_name) {
    printf("[CGROUP] Criando: %s\n", cgroup_name);
    return 0;
}

int cgroup_delete(const char *cgroup_name) {
    printf("[CGROUP] Deletando: %s\n", cgroup_name);
    return 0;
}

int cgroup_add_process(const char *cgroup_name, int pid) {
    printf("[CGROUP] Adicionando PID %d ao: %s\n", pid, cgroup_name);
    return 0;
}

int cgroup_set_limits(const char *cgroup_name, const CgroupLimits *limits) {
    printf("[CGROUP] Configurando limites para: %s\n", cgroup_name);
    return 0;
}

int cgroup_get_metrics(const char *cgroup_name, CgroupMetrics *metrics) {
    printf("[CGROUP] Obtendo métricas de: %s\n", cgroup_name);
    // Preenche com dados dummy para teste
    metrics->cpu_usage = 1000000;
    metrics->memory_usage = 1024 * 1024; // 1MB
    metrics->memory_limit = 100 * 1024 * 1024; // 100MB
    return 0;
}

int cgroup_list_all(char ***cgroups_list, int *count) {
    printf("[CGROUP] Listando todos cgroups...\n");
    *count = 0;
    *cgroups_list = NULL;
    return 0;
}

void cgroup_free_list(char **cgroups_list, int count) {
    printf("[CGROUP] Liberando lista\n");
}

void cgroup_print_metrics(const CgroupMetrics *metrics) {
    printf("[CGROUP] Métricas - CPU: %ld, Mem: %ld/%ld\n", 
           metrics->cpu_usage, metrics->memory_usage, metrics->memory_limit);
}