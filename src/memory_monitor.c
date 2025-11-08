#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

int metricas_swap(int pid, MemMetrics *mem){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/status", pid);
    fp = fopen(proc_path, "r");

    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return -1;
    }

    char buffer[4096];
    char *pt;
    mem -> swap = 0;
    int valor_encontrado = 0;

    

    return 0;
}

int metricas_MEM(int pid, MemMetrics *mem){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/stat", pid);
    fp = fopen(proc_path, "r");

    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return -1;
    }

    char buffer[4096];
    if(fgets(buffer, sizeof(buffer), fp) == NULL){
        printf("Erro ao ler dados do arquivo %s", proc_path);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    char *token;
    int contagem = 1;
    mem -> rss = 0;
    mem -> vsize = 0;
    long minor_faults = 0;
    long major_faults = 0;

    token = strtok(buffer, " ");

    while (token != NULL && contagem <= 24)
    {
        switch (contagem)
        {
        case 10:
            minor_faults = atol(token);
            break;
        case 12:
            major_faults = atol(token);
            break;
        case 23:
            mem -> vsize = atol(token);
            break;
        case 24:
            mem -> rss = atol(token) * 4096;
            break;
        }
        token = strtok(NULL, " ");
        contagem++;
    }
    mem -> page_faults = minor_faults + major_faults;
    
    return 0;
}