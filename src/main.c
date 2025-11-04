#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void monitor_process(int pid){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/stat", pid);
    fp = (fopen(proc_path, "r"));
    if(fp == NULL){
        perror("Erro ao abrir o arquivo /proc");
        return;
    }

    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        fprintf(stderr, "Erro ao ler dados dos arquivos %s\n", proc_path);
        fclose(fp);
        return;
    }

    fclose(fp);

    char *token;
    int contagem = 0;
    long user_time   = 0;
    long system_time = 0;
    long rss = 0;
    long vsz = 0;

    token = strtok(buffer, " ");

    while (token != NULL && contagem <= 24){
        switch (contagem)
        {
        case 14:
            user_time = atol(token);
            break;
        case 15:
            system_time = atol(token);
            break;
        case 23:
            vsz = atol(token);
            break;
        case 24:
            rss = atol(token);
            break;
        }
        token = strtok(NULL, " "); //Continuar a leitura do buffer
        contagem++;
    }
    
    printf("===== Métricas para o PID: %d =====\n", pid);
    printf("User time: %ld\n", user_time);
    printf("System time: %ld\n", system_time);
    printf("Memória virtual: %ld\n", vsz);
    printf("Memória física: %ld", rss);

}

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "Uso: %s <PID>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);

    if (pid <= 0){
        fprintf(stderr, "PID Inválido: %s\n", argv[1]);
        return 1;
    }

    monitor_process(pid);

    return 0;
}