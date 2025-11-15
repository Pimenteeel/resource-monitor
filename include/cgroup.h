
#ifndef CGROUP_H
#define CGROUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>

// Estrutura para métricas de cgroup v1
typedef struct {
    long cpu_usage;          // cpuacct.usage
    long cpu_usage_user;     // cpuacct.usage_user
    long cpu_usage_system;   // cpuacct.usage_system
    long memory_usage;       // memory.usage_in_bytes
    long memory_limit;       // memory.limit_in_bytes
    long memory_failcnt;     // memory.failcnt
    long io_read_bytes;      // blkio.io_service_bytes (Read)
    long io_write_bytes;     // blkio.io_service_bytes (Write)
    long pids_current;       // pids.current
    long pids_limit;         // pids.max
    time_t timestamp;        // Timestamp da coleta
} CgroupMetrics;

// Estrutura para configuração de limites
typedef struct {
    double cpu_limit;        // Limite de CPU em cores (ex: 0.5, 1.0, 2.0)
    long memory_limit;       // Limite de memória em bytes
    long io_read_bps;        // Limite de leitura em bytes/segundo
    long io_write_bps;       // Limite de escrita em bytes/segundo
    long pids_limit;         // Limite máximo de PIDs
} CgroupLimits;

// Estrutura para histórico de métricas
typedef struct {
    CgroupMetrics *metrics;
    int count;
    int capacity;
} CgroupHistory;

#define CGROUP_BASE_PATH "/sys/fs/cgroup"
#define MAX_CGROUP_NAME 256
#define MAX_PATH_LENGTH 512

#endif // CGROUP_H