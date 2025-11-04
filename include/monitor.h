#ifndef MONITOR_H
#define MONITOR_H

typedef struct 
{
    long user_time;
    long system_time;
} CpuMetrics;

typedef struct
{
    long vsize;
    long rss;
} MemMetrics;

typedef struct 
{
    long read_bytes;
    long write_bytes;
} IoMetrics;

int metricas_CPU(int pid, CpuMetrics *cpu);
int metricas_MEM(int pid, MemMetrics *mem);
int metricas_IO(int pid, IoMetrics *io);

#endif