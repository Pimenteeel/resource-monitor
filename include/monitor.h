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
    long vsize; //espaço de endereçamento total que o processo reservou
    long rss; // quantidade de memória ram física que o processo está realmente usando no exato momento
    long page_faults; //quando um programa tenta acessar um endereço de memoria que nao esta na ram
    long swap; //quando a ram fica cheia, o SO pega páginas inativas e guarda Swap OUT
} MemMetrics;

typedef struct 
{
    long read_bytes; // quantidade toal de bytes que o programa pediu o SO 
    long write_bytes; // quantidade total de bytes que o programa enviu para o SO
    long disk_op; //mede a ação física que o disco teve que fazer
    long syscall; // quantas vezes o programa pediu algo
} IoMetrics;

int metricas_CPU(int pid, CpuMetrics *cpu);
int metricas_switches(int pid, CpuMetrics *cpu);

int metricas_MEM(int pid, MemMetrics *mem);
int metricas_swap(int pid, MemMetrics *mem);

int metricas_IO(int pid, IoMetrics *io);

#endif