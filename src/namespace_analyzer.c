#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include "namespace.h"

#define STACK_SIZE (1024 * 64)
char child_stack[STACK_SIZE];
int child_fn(void *arg){
    (void)arg;
    _exit(0);
    return 0;
}

int namespaces_por_pid(int pid, ProcessNamespaces *ns){
    char proc_path[512];
    struct stat buffer; // lista vazia para guardar informações dos status (structure of status) de um arquivo

    // lógica para o mnt
    sprintf(proc_path, "/proc/%d/ns/mnt", pid);
    if (stat(proc_path, &buffer) == -1){
        return -1;
    }
    ns->mnt = (long)buffer.st_ino; //st_ino é o campo que armazena o número do inode (id do ns). o tipo original é ino_t

    // lógica para o uts
    sprintf(proc_path, "/proc/%d/ns/uts", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> uts = 0;
    }
    else{
        ns -> uts = (long)buffer.st_ino;
    }

    // lógica para o ipc
    sprintf(proc_path, "/proc/%d/ns/ipc", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> ipc = 0;
    }
    else{
        ns -> ipc = (long)buffer.st_ino;
    }

    // lógica para o net
    sprintf(proc_path, "/proc/%d/ns/net", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> net = 0;
    }
    else{
        ns -> net = (long)buffer.st_ino;
    }

    // lógica para o pid
    sprintf(proc_path, "/proc/%d/ns/pid", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> pid = 0;
    }
    else{
        ns -> pid = (long)buffer.st_ino;
    }

    // lógica para user
    sprintf(proc_path, "/proc/%d/ns/user", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> user = 0;
    }
    else{
        ns -> user = (long)buffer.st_ino;
    }

    // lógica para cgroup
    sprintf(proc_path, "/proc/%d/ns/cgroup", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> cgroup = 0;
    }
    else{
        ns -> cgroup = (long)buffer.st_ino;
    }

    // lógica para time
    sprintf(proc_path, "/proc/%d/ns/time", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> time = 0;
    }
    else{
        ns -> time = (long)buffer.st_ino;
    }

    return 0;
}

void comparar_namespaces(int pid1, int pid2){
    ProcessNamespaces ns1, ns2;

    if (namespaces_por_pid(pid1, &ns1) != 0){
        fprintf(stderr, "Não foi possível ler namespaces para o PID %d", pid1);
        return;
    }

    if (namespaces_por_pid(pid2, &ns2) != 0){
        fprintf(stderr, "Não foi possível ler namespaces para o PID %d", pid2);
        return;
    }

    printf("Comparando Namespaces: PID %d vs PID %d\n", pid1, pid2);
    printf("TIPO     | PID %d         | PID %d         | IGUAIS?\n", pid1, pid2);
    printf("------------------------------------------------------\n");
    printf("CGROUP   |  %ld      |  %ld      | %s\n", ns1.cgroup, ns2.cgroup, (ns1.cgroup == ns2.cgroup) ? "SIM" : "NAO");
    printf("IPC      |  %ld      |  %ld      | %s\n", ns1.ipc, ns2.ipc, (ns1.ipc == ns2.ipc) ? "SIM" : "NAO");
    printf("MNT      |  %ld      |  %ld      | %s\n", ns1.mnt, ns2.mnt, (ns1.mnt == ns2.mnt) ? "SIM" : "NAO");
    printf("NET      |  %ld      |  %ld      | %s\n", ns1.net, ns2.net, (ns1.net == ns2.net) ? "SIM" : "NAO");
    printf("PID      |  %ld      |  %ld      | %s\n", ns1.pid, ns2.pid, (ns1.pid == ns2.pid) ? "SIM" : "NAO");
    printf("TIME     |  %ld      |  %ld      | %s\n", ns1.time, ns2.time, (ns1.time == ns2.time) ? "SIM" : "NAO");
    printf("USER     |  %ld      |  %ld      | %s\n", ns1.user, ns2.user, (ns1.user == ns2.user) ? "SIM" : "NAO");
    printf("UTS      |  %ld      |  %ld      | %s\n", ns1.uts, ns2.uts, (ns1.uts == ns2.uts) ? "SIM" : "NAO");

}

void mapear_todos_processos(){
    DIR *dir;
    struct dirent *entrada;
    int pid;
    ProcessNamespaces ns;
    char comando_path[512], comando_nome[512];
    FILE *fp, *fp_csv;
    long current_time = time(NULL);

    fp_csv = fopen("Namespace_Report.csv", "w");
    if (fp_csv == NULL){
        perror("Erro ao criar o arquivo de report de namespaces");
        return;
    }
    fprintf(fp_csv, "timestamp,pid,comando,cgroup_id,ipc_id,mnt_id,net_id,pid_id,time_id,user_id,uts_id\n");

    dir = opendir("/proc");
    if (dir == NULL){
        perror("Erro ao abrir o diretório");
        fclose(fp_csv);
        return;
    }
    
    printf("Mapeando todos os procesos por namespaces\n");
    printf("%-10s %-16s %-10s %-10s %-10s\n", "PID", "COMANDO", "NET_ID", "PID_ID", "MNT_ID");
    printf("---------------------------------------------------------------------\n");

    while ((entrada = readdir(dir)) != NULL)
    {  
        if (isdigit(entrada -> d_name[0])){
            pid = atoi(entrada -> d_name);

            if (namespaces_por_pid(pid, &ns) == 0){
                //nome do comando para o relatorio
                sprintf(comando_path, "/proc/%d/comm", pid);
                fp = fopen(comando_path, "r");
                if (fp != NULL){
                    fgets(comando_nome, sizeof(comando_nome), fp);
                    comando_nome[strcspn(comando_nome, "\n")] = 0; // remover o \n
                    fclose(fp);
                }
                else {
                    strcpy(comando_nome, "?"); //mesmo que o fopen falhe, a variável receberá um "?"
                }
                printf("%-10d %-16s %-10ld %-10ld %-10ld\n", pid, comando_nome, ns.net, ns.pid, ns.mnt);

                fprintf(fp_csv, "%ld,%d,%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", current_time, pid,comando_nome, ns.cgroup, ns.ipc, ns.mnt, ns.net, ns.pid, ns.time, ns.user, ns.uts);
            }
        }
    }

    closedir(dir);
    fclose(fp_csv);
    printf("Relatorio de namespaces gerado com sucesso: Namespace_Report.csv\n");
    
}

void namespace_overhead(long iteracao){
    struct timespec start, end;

    printf("Calculando Overhead de namespaces\n");
    printf("Iteracoes: %ld\n", iteracao);

    //teste 1
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iteracao; i++){
        pid_t pid = fork();
        if (pid == 0){
            _exit(0);
        }
        else if (pid > 0){
            waitpid(pid, NULL, 0);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    long tempo_fork = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    printf("\n[1] Tempo total (fork, baseline): %.2f ms\n", (double)tempo_fork / 1000000.0 );

    //teste 2
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iteracao; i++){
        pid_t pid = clone(child_fn, child_stack + STACK_SIZE, CLONE_NEWPID | SIGCHLD, NULL);

        if(pid > 0){
            waitpid(pid, NULL, 0);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    long tempo_clone = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    printf("\n[2] Tempo total (clone + CLONE_NEWPID): %.2f ms\n", (double)tempo_clone / 1000000.0);

    long overhead = tempo_clone - tempo_fork;
    double media_overhead = (double)overhead / (double)iteracao / 1000.0;

    printf("\n-----------------------------------------------\n");
    printf("Overhead de Namespace\n");
    printf("Diferença de tempo total: %.2f ms\n", (double)overhead / 1000000.0);
    printf("Overhead medio por criacao: %.3f us\n", media_overhead);
    printf("\n-----------------------------------------------\n");
}
