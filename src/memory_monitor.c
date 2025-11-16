#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

//Coleta o swap do processo
int metricas_swap(int pid, MemMetrics *mem){ //Valor de swap convertido guardando na MemMetrics
    char proc_path[256];
    FILE *fp;
    
    //monta caminho
    sprintf(proc_path, "/proc/%d/status", pid);
    fp = fopen(proc_path, "r");

    //Caso nao tenha processo ou permissão, erro
    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return -1;
    }

    //Criar buffer para cada linha
    char buffer[4096];
    char *pt;
    mem -> swap = 0;
    int valor_encontrado = 0;

    //Procura VmSwap (indica o uso de swap em kB)
    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        if (valor_encontrado == 0 && strstr(buffer, "VmSwap") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                mem -> swap = atol(pt + 1) * 1024; //Converte KB para byter
                valor_encontrado = 1;
            }
        }
        if(valor_encontrado == 1){
            break;
        }
    }

    fclose(fp);

    return 0;
}

//Coleta as estatisticas de memoria
int metricas_MEM(int pid, MemMetrics *mem){
    char proc_path[256];
    FILE *fp;

    //monta o caminho
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
    mem -> rss = 0;      //momeria fisica
    mem -> vsize = 0;   //Memória virtual  
    long minor_faults = 0;
    long major_faults = 0;

    token = strtok(buffer, " ");

    //percorre até encontrar proc,pid ou stat
    while (token != NULL && contagem <= 24)
    {
        switch (contagem)
        {
        case 10: // minor page faults
            minor_faults = atol(token);
            break;
        case 12: // major page faults
            major_faults = atol(token);
            break;
        case 23: // vsize: memória virtual total em bytes
            mem -> vsize = atol(token);
            break;
        case 24: // rss: memória residente em páginas, converte para bytes
            mem -> rss = atol(token) * 4096; // 4096 = tamanho típico de página
            break;
        }
        token = strtok(NULL, " ");
        contagem++;
    }
    //Soma page faults, menores e maiores
    mem -> page_faults = minor_faults + major_faults;
    
    return 0;
}