#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "monitor.h"
#include "namespace.h"
#include "cgroup.h"

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
        printf("Memória Virtual (VSZ)......: %.2f MB\n", (double)dados_MEM.vsize / (1024 * 1024));
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

        fprintf(log_file, "%ld,%d,%.2f,%.2f,%.2f,%.2f,%ld,%.2f,%.2f,%.2f,%.2f\n",
            (long)time(NULL),
            pid,
            cpu_percent,
            (double)dados_MEM.rss / (1024 * 1024),
            (double)dados_MEM.vsize / (1024 * 1024),
            (double)dados_MEM.swap / (1024 * 1024),
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

static void executar_workload_cpu(int iteracoes) {
    volatile double resultado = 0.0;
    for (int i = 0; i < iteracoes; i++) {
        resultado += (i * 3.14159) / 2.71828;
    }
}

void ajuda_cgroup() {
     printf("\nUso das funções CGroup:\n");
    printf(" -g metrics <cgroup_path>                 Coletar metricas de cgroup\n");
    printf(" -g create <controller> <nome> <cpu> <mem>\n");
    printf("     controller: 'cpu' para limites de CPU\n");
    printf("                 'memory' para limites de Memória\n");
    printf("     cpu: limite em cores (0 = sem limite)\n");
    printf("     mem: limite em MB (0 = sem limite)\n");
    printf(" -g throttle                              Conduzir experimentos de throttling\n");
    printf(" -g memlimit                              Conduzir experimento de limite de memoria\n");
    printf(" -g report <cgroup_path>                  Gerar relatorio de utilizacao\n");
}

void conduzir_experimentos_throttling(void) {
    printf("EXPERIMENTO 3: THROTTLING DE CPU\n");
    printf("=================================\n\n");
    
    double limites[] = {0.25, 0.5, 1.0, 2.0};
    int num_limites = sizeof(limites) / sizeof(limites[0]);
    
    printf("LIMITE (cores) | CPU%% MEDIDO | THROUGHPUT (iter/s) | DESVIO%%\n");
    printf("---------------------------------------------------------------\n");
    
    for (int i = 0; i < num_limites; i++) {
        double limite = limites[i];
        char cgroup_name[64];
        snprintf(cgroup_name, sizeof(cgroup_name), "throttle_%.2f", limite);
        
        if (!criar_configurar_cgroup("cpu", cgroup_name, limite, 0)) {
            continue;
        }
        
        char cgroup_path[MAX_PATH];
        int written = snprintf(cgroup_path, sizeof(cgroup_path), "/sys/fs/cgroup/%s", cgroup_name);
        if (written >= (int)sizeof(cgroup_path)) {
            fprintf(stderr, "Caminho do cgroup muito longo\n");
            continue;
        }
        
        char metric_path[MAX_PATH];
        written = snprintf(metric_path, sizeof(metric_path), "%s/cpu.stat", cgroup_path);
        if (written >= (int)sizeof(metric_path)) {
            fprintf(stderr, "Caminho da metrica CPU muito longo\n");
            continue;
        }
        
        unsigned long long uso_cpu_antes = 0;
        FILE* cpu_file = fopen(metric_path, "r");
        if (cpu_file) {
            char line[256];
            while (fgets(line, sizeof(line), cpu_file)) {
                if (strstr(line, "usage_usec")) {
                    sscanf(line, "usage_usec %llu", &uso_cpu_antes);
                    uso_cpu_antes *= 1000; // converter para nanosegundos
                }
            }
    fclose(cpu_file);
        }
        
        if (!move_process_to_cgroup(cgroup_path, getpid())) {
            continue;
        }
        
        struct timespec inicio, fim;
        clock_gettime(CLOCK_MONOTONIC, &inicio);
        
        int iteracoes = 50000000;
        executar_workload_cpu(iteracoes);
        
        clock_gettime(CLOCK_MONOTONIC, &fim);
        
        double tempo_decorrido_ns = (fim.tv_sec - inicio.tv_sec) * 1e9 + (fim.tv_nsec - inicio.tv_nsec);
        double tempo_decorrido_s = tempo_decorrido_ns / 1e9;
        
        unsigned long long uso_cpu_depois = 0;
        cpu_file = fopen(metric_path, "r");
        if (cpu_file) {
            char line[256];
            while (fgets(line, sizeof(line), cpu_file)) {
                if (strstr(line, "usage_usec")) {
                    sscanf(line, "usage_usec %llu", &uso_cpu_depois);
                    uso_cpu_depois *= 1000; // converter para nanosegundos
                }
            }
    fclose(cpu_file);
        }
        
        unsigned long long delta_cpu_ns = uso_cpu_depois - uso_cpu_antes;
        double cpu_percent = (delta_cpu_ns / tempo_decorrido_ns) * 100.0;
        
        double throughput = iteracoes / tempo_decorrido_s;
        
        double desvio_percentual = ((cpu_percent - (limite * 100)) / (limite * 100)) * 100;
        
        printf("%.2f           | %-10.1f  | %-18.0f  | %-7.1f\n", 
               limite, cpu_percent, throughput, desvio_percentual);
        
        char procs_path[MAX_PATH];
        written = snprintf(procs_path, sizeof(procs_path), "%s/cgroup.procs", cgroup_path);
        if (written < (int)sizeof(procs_path)) {
            write_to_file(procs_path, "0");
        }
        
        sleep(1);
        rmdir(cgroup_path);
        
        if (i < num_limites - 1) sleep(1);
    }
    
    printf("---------------------------------------------------------------\n");
    printf("Experimento de throttling concluido\n");
}

void experimento_limite_memoria(void) {
    printf("\nEXPERIMENTO 4: LIMITACAO DE MEMORIA\n");
    printf("==================================\n");
    
    printf("Procedimento:\n");
    printf("1. Criar cgroup com limite de 100MB\n");
    printf("2. Tentar alocar memoria incrementalmente\n");
    printf("3. Observar comportamento (OOM killer, falhas de alocacao)\n\n");
    
    if (!criar_configurar_cgroup("memory", "exp_memoria", 0, 100)) {
        return;
    }
    
    char cgroup_path[MAX_PATH];
    int written = snprintf(cgroup_path, sizeof(cgroup_path), "/sys/fs/cgroup/exp_memoria");
    if (written >= (int)sizeof(cgroup_path)) {
        fprintf(stderr, "Caminho do cgroup memoria muito longo\n");
        return;
    }
    
    if (!move_process_to_cgroup(cgroup_path, getpid())) {
        return;
    }
    
    printf("TESTE DE ALOCACAO DE MEMORIA:\n");
    printf("Tentativa | Tamanho (MB) | Status\n");
    printf("-----------------------------------\n");
    
    size_t tamanhos_mb[] = {10, 25, 50, 75, 100, 110, 120};
    int num_testes = sizeof(tamanhos_mb) / sizeof(tamanhos_mb[0]);
    
    size_t max_alocado = 0;
    int falhas = 0;
    
    for (int i = 0; i < num_testes; i++) {
        size_t tamanho_mb = tamanhos_mb[i];
        size_t tamanho_bytes = tamanho_mb * 1024 * 1024;
        
        void* memoria = malloc(tamanho_bytes);
        
        if (memoria != NULL) {
            memset(memoria, 0xFF, tamanho_bytes);
            
            printf("%-9d | %-12zu | ALOCADO\n", i + 1, tamanho_mb);
            max_alocado = tamanho_mb;
            free(memoria);
        } else {
            printf("%-9d | %-12zu | FALHA\n", i + 1, tamanho_mb);
            falhas++;
            break;
        }
        
        char metric_path[MAX_PATH];
        written = snprintf(metric_path, sizeof(metric_path), "%s/memory.current", cgroup_path);
        if (written < (int)sizeof(metric_path)) {
            long memoria_atual = read_from_file_long(metric_path);
            printf("           |              | Memoria atual: %.2f MB\n", 
                   (double)memoria_atual / (1024 * 1024));
        }
        
        sleep(1);
    }
    
    char metric_path[MAX_PATH];
    int written_failcnt = snprintf(metric_path, sizeof(metric_path), "%s/memory.events", cgroup_path);
    long failcnt = 0;
    if (written_failcnt < (int)sizeof(metric_path)) {
        failcnt = read_from_file_long(metric_path);
    }
    
    printf("\nRESULTADOS FINAIS:\n");
    printf("----------------------------------------\n");
    printf("Quantidade maxima alocada: %zu MB\n", max_alocado);
    printf("Numero de falhas: %d\n", falhas);
    printf("Contador de falhas (memory.failcnt): %ld\n", failcnt);
    printf("Comportamento: %s\n", 
           falhas > 0 ? "Falha de alocacao ao atingir limite" : "Todas alocacoes bem-sucedidas");
    printf("----------------------------------------\n");
    
    char procs_path[MAX_PATH];
    written = snprintf(procs_path, sizeof(procs_path), "%s/cgroup.procs", cgroup_path);
    if (written < (int)sizeof(procs_path)) {
        write_to_file(procs_path, "0");
    }
    sleep(1);
    rmdir(cgroup_path);
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
        fprintf(stderr, " -g Funções relacionadas a CGroup\n");
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
    else if (strcmp(flag, "-g") == 0) {
    if (argc < 3) {
        ajuda_cgroup();
        return 1;
    }

    char* subcomando = argv[2];

    if (strcmp(subcomando, "metrics") == 0) {
        if (argc == 4) {
            coletar_metricas_cgroup(argv[3]);
        } else {
            printf("Erro: Uso: -g metrics <cgroup_path>\n");
            ajuda_cgroup();
            return 1;
        }
    }
    else if (strcmp(subcomando, "create") == 0 && argc == 7) {
        char* nome = argv[4];
        double cpu = atof(argv[5]);
        long mem = atol(argv[6]);
        
        // Criar cgroup CPU se especificado
        if (cpu > 0) {
            if (!criar_configurar_cgroup("cpu", nome, cpu, 0)) {
                fprintf(stderr, "Erro ao criar cgroup CPU %s\n", nome);
            } else {
                printf(" Cgroup CPU criado: /sys/fs/cgroup/%s\n", nome);
            }
        }
        
        // Criar cgroup Memory se especificado
        if (mem > 0) {
            if (!criar_configurar_cgroup("memory", nome, 0, mem)) {
                fprintf(stderr, "Erro ao criar cgroup Memory %s\n", nome);
            } else {
                printf(" Cgroup Memory criado: /sys/fs/cgroup/%s\n", nome);
            }
        }
    }
    else if (strcmp(subcomando, "throttle") == 0) {
        conduzir_experimentos_throttling();
    }
    else if (strcmp(subcomando, "memlimit") == 0) {
        experimento_limite_memoria();
    }
    else if (strcmp(subcomando, "report") == 0) {
        if (argc == 4) {
            gerar_relatorio_utilizacao(argv[3]);
        } else {
            printf("Erro: Uso: -g report <cgroup_path>\n");
            ajuda_cgroup();
            return 1;
        }
    }
    else {
        ajuda_cgroup();
        return 1;
    }
}

    return 0;
}