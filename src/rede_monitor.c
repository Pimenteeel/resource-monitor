#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

int metricas_REDE(int pid, RedeMetrics *red){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/net/dev", pid);
    fp = fopen(proc_path, "r");

    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return -1;
    }

    char buffer[4096];
    char *pt;
    red -> bytes_rx = 0;
    red -> bytes_tx = 0;
    red -> packets_rx = 0;
    red -> packets_tx = 0;

    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        if (strstr(buffer, "eth0") != NULL){
            pt = strchr(buffer, ':');
            if (pt != NULL){

                char *token;

                token = strtok(pt + 1, " ");
                if (token != NULL){
                    red -> bytes_rx = atol(token);
                }

                token = strtok(NULL, " ");
                if (token != NULL){
                    red -> packets_rx = atol(token);
                }

                for (int i = 3; i <= 8; i++){
                    token = strtok(NULL, " ");
                    if (token == NULL){
                        break;
                    }
                }

                if (token != NULL){
                    token = strtok(NULL, " ");
                    if (token != NULL){
                        red -> bytes_tx = atol(token);
                    }
                }

                if (token != NULL){
                    token = strtok(NULL, " ");
                    if (token != NULL){
                        red -> packets_tx = atol(token);
                    }
                }
                break;
            }
        }
    }
    fclose(fp);
    return 0;
}