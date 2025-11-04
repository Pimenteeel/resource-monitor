#include <stdio.h>
#include <stdlib.h>

void monitor_process(int pid) {
    char proc_path[256];
    FILE *fp;

    // 1. Constrói o caminho para o arquivo /proc/[pid]/stat
    //    snprintf é uma forma segura de formatar strings.
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/stat", pid);

    printf("Tentando abrir: %s\n", proc_path);

    // 2. Tenta abrir o arquivo para leitura ("r")
    fp = fopen(proc_path, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo /proc");
        return;
    }

    // 3. Lê e imprime o conteúdo
    char buffer[4096];
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("\nConteúdo do /proc/%d/stat:\n%s\n", pid, buffer);
    }

    // 4. Fecha o arquivo
    fclose(fp);
}

int main(int argc, char *argv[]) {
    // argc é a contagem de argumentos. argv são os argumentos.
    // argv[0] é o nome do programa (ex: ./resource-monitor)
    // argv[1] será o nosso PID

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PID>\n", argv[0]);
        return 1; // Retorna 1 para indicar erro
    }

    // Converte o primeiro argumento (string) para um inteiro (int)
    int pid = atoi(argv[1]);

    if (pid <= 0) {
        fprintf(stderr, "PID inválido: %s\n", argv[1]);
        return 1;
    }

    monitor_process(pid);

    return 0; // Sucesso
}