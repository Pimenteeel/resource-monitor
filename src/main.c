#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "monitor.h"
#include "namespace.h"

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

    FILE *log_file = fopen("Resource_Monitor.csv", "w");
    if (log_file == NULL){
        perror("Não foi possível criar o arquivo de log .csv");
        return;
    }
    fprintf(log_file, "timestamp,pid,cpu_percent,rss_mb,vsz_mb,swap_mb,total_page_faults,io_read_rate_mbs,io_write_rate_mbs,net_rx_rate_mbs,net_tx_rate_mbs\n");

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

        fprintf(log_file, "%ld,%d,%.2f,%.2ld,%.2ld,%.2ld,%ld,%.2f,%.2f,%.2f,%.2f\n",
            (long)time(NULL),
            pid,
            cpu_percent,
            dados_MEM.rss,
            dados_MEM.vsize,
            dados_MEM.swap,
            dados_MEM.page_faults,
            taxa_leitura_mbs,
            taxa_escrita_mbs,
            taxa_rede_rx_mbs,
            taxa_rede_tx_mbs
        );
        fflush(log_file);

        dados_CPU_ante = dados_CPU;
        dados_IO_ante = dados_IO;
        dados_REDE_ante = dados_REDE;
        ticks_sistema_ante = ticks_sistema_agora;

        sleep(INTERVALO);
    }

    fclose(log_file);
    printf("\nMonitoramento encerrado. Log salvo em Resource_Monitor.csv\n");
}

void monitorar_namespaces(int pid){

    ProcessNamespaces ns;

    if(namespaces_por_pid(pid, &ns) != 0){
        fprintf(stderr, "Não foi possível ler namespaces para o PID %d", pid);
        return;
    }
    printf("Listando Namespaces para o PID: %d\n", pid);
    printf("TIPO     | ID (INODE)\n");
    printf("---------------------------\n");
    printf("CGROUP   | %ld\n", ns.cgroup);
    printf("IPC      | %ld\n", ns.ipc);
    printf("MNT      | %ld\n", ns.mnt);
    printf("NET      | %ld\n", ns.net);
    printf("PID      | %ld\n", ns.pid);
    printf("TIME     | %ld\n", ns.time);
    printf("USER     | %ld\n", ns.user);
    printf("UTS      | %ld\n", ns.uts);

}

int main(int argc, char *argv[]){
    if (argc < 2){
        fprintf(stderr, "Uso: %s <flag> <PID> [PID]\n", argv[0]);
        fprintf(stderr, "Flags:\n");
        fprintf(stderr, " -r Monitorar Recursos (loop)\n");
        fprintf(stderr, " -n Listar Namespaces\n");
        fprintf(stderr, " -c <PID1> <PID2> Comparar Namespaces\n");
        fprintf(stderr, " -m Mapear todos os processos\n");
        fprintf(stderr, " -o Medir overhead de Namespaces\n");
        return 1;
    }


    char *flag = argv[1];

    if (strcmp(flag, "-r") == 0 || strcmp(flag, "-n") == 0){
        
        if (argc != 3){
            fprintf(stderr, "Erro: As flags -r e -n exigem exatamente 1 PID\n");
            return 1;
        }

        int pid = atoi(argv[2]);
        if (pid <= 0){
            fprintf(stderr, "PID inválido: %s\n", argv[2]);
            return 1;
        }

        if (strcmp(flag, "-r") == 0){
            monitor_process(pid);
        }
        else {
            monitorar_namespaces(pid);
        }
    }
    else if (strcmp(flag, "-c") == 0){

        if (argc != 4){
            fprintf(stderr, "Erro: A flag -c exige exatamente 2 PIDs\n");
            return 1;
        }

        int pid1 = atoi(argv[2]);
        int pid2 = atoi(argv[3]);

        if (pid1 <= 0 || pid2 <= 0){
            fprintf(stderr, "PIDs inválidos: %s e %s\n", argv[2], argv[3]);
            return 1;
        }

        comparar_namespaces(pid1, pid2);
    }
    else if (strcmp(flag, "-m") == 0){

        if (argc != 2){
            fprintf(stderr, "Erro: A Flag -m não exige PIDs\n");
            return 1;
        }
        mapear_todos_processos();
    }
    else if (strcmp(flag, "-o") == 0){
        if (argc != 3){
            fprintf(stderr, "Erro: A flag -o exige numero de iteracoes\n");
            fprintf(stderr, "Uso: %s -o <ITERACOES>\n", argv[0]);
            return 1;
        }

        long iteracao = atol(argv[2]);

        if (iteracao <= 0){
            fprintf(stderr, "Numero de iteracoes invalido: %s\n", argv[2]);
            return 1;
        }

        namespace_overhead(iteracao);
    }
    else {
        fprintf(stderr, "Flag inválida: %s\n", flag);
    }
<<<<<<< HEAD
=======


>>>>>>> main
    return 0;
}