#include <stdio.h>
#include <stdlib.h>

void monitor_process(int pid){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/status", pid);
    fp = (fopen(proc_path, "r"));
    if(fp == NULL){
        perror("Erro ao abrir o arquivo /proc");
        return;
    }

    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        printf("\nConteúdo do /proc/%d/status: \n%s\n", pid, buffer);
    }
    
    fclose(fp);
}

int main(int argc, char *argv[]){
    
}