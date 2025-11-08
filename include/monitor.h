#ifndef MONITOR_H
#define MONITOR_H

typedef struct 
{
    long user_time; // tempo que a cpu gasta executando o código do programa
    long system_time; // tempo que a cpu gasta executando o código do sistema operacional em nome do programa
    long threads; // menor unidade de execução dentro de um programa
    long switches; // processo que o sistema operacional usa para pausar um programa e começa a executar outro
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
int metricas_switches(int pid, CpuMetrics *cpu);
int metricas_MEM(int pid, MemMetrics *mem);
int metricas_IO(int pid, IoMetrics *io);

#endif