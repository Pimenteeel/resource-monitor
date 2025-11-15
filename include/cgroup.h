
#ifndef CGROUP_H
#define CGROUP_H

#include "cgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // Para sleep, rmdir
#include <sys/stat.h>    // Para mkdir
#include <sys/types.h>
#include <time.h>        // Para time()
#include <errno.h>       // Para tratamento de erros
#include <dirent.h>      // Para listar diretórios 


// Estrutura para armazenar os dados lidos do Kernel
typedef struct {
    long cpu_usage;          //calcular o consumo total de CPU
    long cpu_usage_user;     
    long cpu_usage_system;   // overhead do Kernel.
    long memory_usage;       
    long memory_limit;       
    long memory_failcnt;     //quantas vezes o limite de memória foi atingido
    long io_read_bytes;     //no disco
    long io_write_bytes;     //no disco
    long pids_current;       // numero de processos/threads
    long pids_limit;         
    time_t timestamp;       //Momento exato da coleta de dados.
} CgroupMetrics;

// Estrutura para configuração de limites
typedef struct {
    double cpu_limit;         
    long memory_limit;       
    long io_read_bps;        
    long io_write_bps;        
    long pids_limit;         
} CgroupLimits;

// Estrutura para histórico de métricas
typedef struct {
    CgroupMetrics *metrics; 
    int count; // quantos pontos de dados foram salvos
    int capacity; // Capacidade máxima atual do array alocado
} CgroupHistory;

#define CGROUP_BASE_PATH "/sys/fs/cgroup"

int cgroup_create(const char *cgroup_name);
int cgroup_delete(const char *cgroup_name);
int cgroup_add_process(const char *cgroup_name, int pid);
int cgroup_set_limits(const char *cgroup_name, const CgroupLimits *limits);
int cgroup_get_metrics(const char *cgroup_name, CgroupMetrics *metrics);
int cgroup_list_all(char ***cgroups_list, int *count);
void cgroup_free_list(char **cgroups_list, int count);
void cgroup_print_metrics(const CgroupMetrics *metrics);

#endif // CGROUP_H