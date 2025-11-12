#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "monitor.h"

#define INTERVALO 1

void monitor_process(int pid){
    
    CpuMetrics dados_CPU;
    MemMetrics dados_MEM;
    IoMetrics dados_IO;
    RedeMetrics dados_REDE;

    CpuMetrics dados_CPU_ante;
    IoMetrics dados_IO_ante;
    RedeMetrics dados_REDE_ante;
    long ticks_sistema_ante;

    metricas_CPU(pid, &dados_CPU_ante);
    metricas_IO(pid, &dados_IO_ante);
    metricas_REDE(pid, &dados_REDE_ante);
    ticks_sistema_ante = total_ticks_sistema();

    sleep(INTERVALO);

    while(1){

        system("clear");

        if(metricas_CPU(pid, &dados_CPU) != 0){
            fprintf(stderr, "Não foi possível ler metricas de CPU para o PID %d\n", pid);
            return;
        }

        if(metricas_switches(pid, &dados_CPU) != 0){
            fprintf(stderr, "Não foi possível ler metricas de switches para o PID %d\n", pid);
            return;
        }

        if(metricas_MEM(pid, &dados_MEM) != 0){
            fprintf(stderr, "Não foi possível ler metricas de memória para o PID %d\n", pid);
            return;
        }

        if(metricas_swap(pid, &dados_MEM) != 0){
            fprintf(stderr, "Não foi possível ler metricas de swap para o PID %d\n", pid);
            return;
        }

        if(metricas_IO(pid, &dados_IO) != 0){
            fprintf(stderr, "Não foi possível ler metricas de I/O para o PID %d\n", pid);
            return;
        }

        if(metricas_REDE(pid, &dados_REDE) != 0){
            fprintf(stderr, "Não foi possível ler metricas de Rede para o PID %d\n", pid);
            return;
        }

        long ticks_sistema_agora = total_ticks_sistema();

        printf("==================== Métricas para o PID %d ====================\n", pid);
        printf("--- Métricas de CPU: ---\n");
        printf("User time..................: %ld clock ticks\n", dados_CPU.user_time);
        printf("System time................: %ld clock ticks\n", dados_CPU.system_time);
        printf("Threads....................: %ld\n", dados_CPU.threads);
        printf("Context Switches...........: %ld\n", dados_CPU.switches);
        printf("\n");
        printf("--- Métricas de memória: ---\n");
        printf("Memória Virtual (VSZ)......: %.2f MB\n", (double)dados_MEM.vsize / (1024 * 1024)); //o double pega as casas decimais para o retorno
        printf("Memória Física (RSS).......: %.2f MB\n", (double)dados_MEM.rss / (1024 * 1024));
        printf("Memória em Swap............: %.2f MB\n", (double)dados_MEM.swap / (1024 * 1024));
        printf("Total de Page Faults.......: %ld\n", dados_MEM.page_faults);
        printf("\n");
        printf("--- Métricas de I/O: ---\n");
        printf("Total de Syscalls..........: %ld\n", dados_IO.syscall);
        printf("Total de Disk Operations...: %ld\n", dados_IO.disk_op);
        printf("Total de bytes read........: %.2f MB\n", (double)dados_IO.read_bytes / (1024 * 1024));
        printf("Total de bytes write.......: %.2f MB\n", (double)dados_IO.write_bytes / (1024 * 1024));
        printf("\n");
        printf("--- Métricas de Rede: ---\n");
        printf("Bytes recebidos............: %ld Bytes\n", dados_REDE.bytes_rx);
        printf("Bytes transmitidos.........: %ld Bytes\n", dados_REDE.bytes_tx);
        printf("Pacotes recebidos..........: %ld Pacotes\n", dados_REDE.packets_rx);
        printf("Pacotes transmitidos.......: %ld Pacotes\n", dados_REDE.packets_tx);

        long delta_proc = (dados_CPU.user_time + dados_CPU.system_time) - (dados_CPU_ante.user_time + dados_CPU_ante.system_time);
        long delta_sistema = ticks_sistema_agora - ticks_sistema_ante;

        double cpu_percent = 0.0;
        if (delta_sistema > 0){
            cpu_percent = 100.0 * (double)delta_proc / (double)delta_sistema;
        }

        long delta_read = dados_IO.read_bytes - dados_IO_ante.read_bytes;
        long delta_write = dados_IO.write_bytes - dados_IO_ante.write_bytes;

        double taxa_leitura_mbs = ( (double)delta_read / (1024*1024)) / INTERVALO;
        double taxa_escrita_mbs = ( (double)delta_write / (1024*1024)) / INTERVALO;

        long delta_rede_rx = dados_REDE.bytes_rx - dados_REDE_ante.bytes_rx;
        long delta_rede_tx = dados_REDE.bytes_tx - dados_REDE_ante.bytes_tx;

        double taxa_rede_rx_mbs = ( (double)delta_rede_rx / (1024*1024) ) / INTERVALO;
        double taxa_rede_tx_mbs = ( (double)delta_rede_tx / (1024*1024) ) / INTERVALO;

        printf("\n--- TAXAS (por %d segundo) ---\n", INTERVALO);
        printf("Uso de CPU..............: %.2f %%\n", cpu_percent);
        printf("Taxa de Leitura (I/O)...: %.2f MB/s\n", taxa_leitura_mbs);
        printf("Taxa de Escrita (I/O)...: %.2f MB/s\n", taxa_escrita_mbs);
        printf("Taxa de Rede (RX).......: %.2f MB/s\n", taxa_rede_rx_mbs);
        printf("Taxa de Rede (TX).......: %.2f MB/s\n", taxa_rede_tx_mbs);

        dados_CPU_ante = dados_CPU;
        dados_IO_ante = dados_IO;
        dados_REDE_ante = dados_REDE;
        ticks_sistema_ante = ticks_sistema_agora;

        sleep(INTERVALO);
    }
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