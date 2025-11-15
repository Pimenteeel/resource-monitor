#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include "namespace.h"

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
    FILE *fp;

    dir = opendir("/proc");
    if (dir == NULL){
        perror("Erro ao abrir o diretório");
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
            }
        }
    }

    closedir(dir);
    
}