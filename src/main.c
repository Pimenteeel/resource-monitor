#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

void monitor_process(int pid){
    
    CpuMetrics dados_CPU;
    MemMetrics dados_MEM;

    if(metricas_CPU(pid, &dados_CPU) != 0){
        fprintf(stderr, "Não foi possível ler metricas de CPU para o PID %d\n", pid);
        return;
    }

    if(metricas_switches(pid, &dados_CPU) != 0){
        fprintf(stderr, "Não foi possível ler metricas de switches para o PID %d\n", pid);
        return;
    }

    if(metricas_MEM(pid, &dados_MEM) != 0){
        fprintf(stderr, "Não foi possível ler metricas de CPU para o PID %d\n", pid);
        return;
    }

    if(metricas_swap(pid, &dados_MEM) != 0){
        fprintf(stderr, "Não foi possível ler metricas de swap para o PID %d\n", pid);
        return;
    }

    printf("========== Métricas para o PID %d ==========\n", pid);
    printf("--- Métricas de CPU: ---\n");
    printf("User time.......: %ld clock ticks\n", dados_CPU.user_time);
    printf("System time.....: %ld clock ticks\n", dados_CPU.system_time);
    printf("Threads.........: %ld\n", dados_CPU.threads);
    printf("Context Switches: %ld\n", dados_CPU.switches);
    printf("\n");
    printf("--- Métricas de memória: ---\n");
    printf("Memória Virtual (VSZ): %.2f MB\n", (double)dados_MEM.vsize / (1024 * 1024)); //o double pega as casas decimais para o retorno
    printf("Memória Física (RSS).: %.2f MB\n", (double)dados_MEM.rss / (1024 * 1024));
    printf("Memória em Swap......: %.2f MB\n", (double)dados_MEM.swap / (1024 * 1024));
    printf("Total de Page Faults.: %ld\n", dados_MEM.page_faults);

}

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "Uso: %s <PID>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);

    if (pid <= 0){
        fprintf(stderr, "PID Inválido: %s\n", argv[1]);
        return 1;
    }

    monitor_process(pid);

    return 0;
}