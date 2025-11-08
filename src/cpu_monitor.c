#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

int metricas_switches(int pid, CpuMetrics *cpu){
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
    long voluntary = 0;
    long nonvoluntary = 0;

    int valor_encontrado_voluntary = 0;
    int valor_encontrado_nonvoluntary = 0;


    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        if (valor_encontrado_voluntary == 0 && strstr(buffer, "voluntary_ctxt_switches") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                voluntary = atol(pt + 1);
                valor_encontrado_voluntary = 1;
            }
        }
        if (valor_encontrado_nonvoluntary == 0 && strstr(buffer, "nonvoluntary_ctxt_switches") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                nonvoluntary = atol(pt + 1);
                valor_encontrado_nonvoluntary = 1;
            }
        }
        if (valor_encontrado_voluntary == 1 && valor_encontrado_nonvoluntary == 1){
            break;
        }
    }

    fclose(fp);

    cpu -> switches = voluntary + nonvoluntary;

    return 0;
}

int metricas_CPU(int pid, CpuMetrics *cpu){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/stat", pid);
    fp = fopen(proc_path, "r");

    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return -1;
    }

    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        printf("Erro ao ler dados do arquivo %s", proc_path);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    char *token; //Para percorrer o buffer
    int contagem = 1;
    cpu -> user_time = 0;
    cpu -> system_time = 0;
    cpu -> threads = 0;

    token = strtok(buffer, " ");

    while (token != NULL && contagem <= 20){
        switch (contagem)
        {
        case 14:
            cpu -> user_time = atol(token);
            break;
        case 15:
            cpu -> system_time = atol(token);
            break;
        case 20:
            cpu -> threads = atol(token);
            break;
        }
        token = strtok(NULL, " ");
        contagem++;
    }

    return 0;
}