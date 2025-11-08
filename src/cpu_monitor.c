#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

int metricas_CPU(int pid, CpuMetrics *cpu){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/stat", pid);
    fp = fopen(proc_path, "r");

    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return;
    }

    char buffer[4096];
    fgets(buffer, sizeof(buffer), fp);
    if (buffer == NULL)
    {
        printf("Erro ao ler dados do arquivo %s", proc_path);
        fclose(fp);
        return;
    }

    fclose(fp);

    char *token; //Para percorrer o buffer
    int contagem = 1;
    cpu -> user_time = 0;
    cpu -> system_time = 0;
    cpu -> threads = 0;

    token = strtok(buffer, " ");

    while (token != NULL && contagem <= 15){
        switch (contagem)
        {
        case 14:
            cpu -> user_time = atol(token);
        case 15:
            cpu -> system_time = atol(token);
        case 20:
            cpu -> threads = atol(token);
        }
        token = strtok(NULL, " ");
        contagem++;
    }

    return 0;
}