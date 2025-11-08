#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

int metricas_IO(int pid, IoMetrics *io){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/io", pid);
    fp = fopen(proc_path , "r");

    if(fp == NULL){
        perror("Erro ao abrir o processo");
        return -1;
    }

    char buffer[4096];
    char *pt;
    int vlr_rb = 0, vlr_wb = 0, vlr_diskop_l = 0, vlr_diskop_e = 0, vlr_sys_l = 0, vlr_sys_e = 0;
    io -> read_bytes = 0;
    io -> write_bytes = 0;
    long diskop_leitura = 0;
    long diskop_escrita = 0;
    long syscall_leitura = 0;
    long syscall_escrita = 0;

    while(fgets(buffer, sizeof(buffer), fp) != NULL){
        // read_bytes
        if(vlr_rb == 0 && strstr(buffer, "rchar") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                io -> read_bytes = atol(pt + 1);
                vlr_rb = 1;
            }
        }
        // write_bytes
        if(vlr_wb == 0 && strstr(buffer, "wchar") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                io -> write_bytes = atol(pt + 1);
                vlr_wb = 1;
            }
        }
        // disk_op_leitura
        if(vlr_diskop_l == 0 && strstr(buffer, "read_bytes") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                diskop_leitura = atol(pt + 1);
                vlr_diskop_l = 1;
            }
        }
        // disk_op_escrita
        if(vlr_diskop_e == 0 && strstr(buffer, "write_bytes") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                diskop_escrita = atol(pt + 1);
                vlr_diskop_e = 1;
            }
        }
        // syscall_leitura
        if(vlr_sys_l == 0 && strstr(buffer, "syscr") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                syscall_leitura = atol(pt + 1);
                vlr_sys_l = 1;
            }
        }
        // syscall_escrita
        if(vlr_sys_e == 0 && strstr(buffer, "syscw") != NULL){
            pt = strchr(buffer, ':');
            if(pt != NULL){
                syscall_escrita = atol(pt + 1);
                vlr_sys_e = 1;
            }
        }
        if(vlr_diskop_e == 1 && vlr_diskop_l == 1 && vlr_rb == 1 && vlr_sys_e == 1 && vlr_sys_l == 1 && vlr_wb == 1){
            break;
        }
    }

    fclose(fp);

    io -> disk_op = diskop_escrita + diskop_leitura;
    io -> syscall = syscall_escrita + syscall_leitura;

    return 0;
}