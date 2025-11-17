
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "monitor.h"

//Coleta metricas das I/O do processo
int metricas_IO(int pid, IoMetrics* io) {
    char proc_path[256];
    FILE* fp;

    //Monta o caminho proc,PID, io
    sprintf(proc_path, "/proc/%d/io", pid);
    fp = fopen(proc_path, "r");

    //Caso nao tenha processo ou permissão, erro
    if (fp == NULL) {
        perror("Erro ao abrir o processo");
        return -1;
    }

    //Criar buffer para cada linha, pesquisar
    char buffer[4096];
    char* pt;
    int vlr_rb = 0, vlr_wb = 0, vlr_diskop_l = 0, vlr_diskop_e = 0, vlr_sys_l = 0, vlr_sys_e = 0;
    io->read_bytes = 0;
    io->write_bytes = 0;
    long diskop_leitura = 0;
    long diskop_escrita = 0;
    long syscall_leitura = 0;
    long syscall_escrita = 0;

    //Procura métricas linha por linha
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // captura rcha 
        if (vlr_rb == 0 && strstr(buffer, "rchar") != NULL) {
            pt = strchr(buffer, ':');
            if (pt != NULL) {
                io->read_bytes = atol(pt + 1);
                vlr_rb = 1;
            }
        }
        // captura wchar
        if (vlr_wb == 0 && strstr(buffer, "wchar") != NULL) {
            pt = strchr(buffer, ':');
            if (pt != NULL) {
                io->write_bytes = atol(pt + 1);
                vlr_wb = 1;
            }
        }
        // captura read_bytes(Bytes lidos do disco)
        if (vlr_diskop_l == 0 && strstr(buffer, "read_bytes") != NULL) {
            pt = strchr(buffer, ':');
            if (pt != NULL) {
                diskop_leitura = atol(pt + 1);
                vlr_diskop_l = 1;
            }
        }
        // captura writhe_bytes
        if (vlr_diskop_e == 0 && strstr(buffer, "write_bytes") != NULL) {
            pt = strchr(buffer, ':');
            if (pt != NULL) {
                diskop_escrita = atol(pt + 1);
                vlr_diskop_e = 1;
            }
        }
        // captura o syscr
        if (vlr_sys_l == 0 && strstr(buffer, "syscr") != NULL) {
            pt = strchr(buffer, ':');
            if (pt != NULL) {
                syscall_leitura = atol(pt + 1);
                vlr_sys_l = 1;
            }
        }
        // syscall_escrita
        if (vlr_sys_e == 0 && strstr(buffer, "syscw") != NULL) {
            pt = strchr(buffer, ':');
            if (pt != NULL) {
                syscall_escrita = atol(pt + 1);
                vlr_sys_e = 1;
            }
        }
        //Sai do loop caso todas metricas capturadas
        if (vlr_diskop_e == 1 && vlr_diskop_l == 1 && vlr_rb == 1 && vlr_sys_e == 1 && vlr_sys_l == 1 && vlr_wb == 1) {
            break;
        }
    }

    fclose(fp);
    //calcula os totais de operações no disco e syscalls
    io->disk_op = diskop_escrita + diskop_leitura;
    io->syscall = syscall_escrita + syscall_leitura;

    return 0;
}
