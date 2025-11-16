#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_SIZE (500 * 1024 * 1024) // 500MB
#define BUFFER_SIZE (4 * 1024) // 4KB
#define NUM_FILES 5

void create_large_file(const char* filename, size_t size) {
    printf("Criando arquivo %s (%ld MB)...\n", filename, size / (1024 * 1024));
    
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Erro ao criar arquivo");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 'X', BUFFER_SIZE); // Preencher buffer com dados
    
    size_t written = 0;
    while (written < size) {
        ssize_t bytes = write(fd, buffer, BUFFER_SIZE);
        if (bytes == -1) {
            perror("Erro ao escrever no arquivo");
            break;
        }
        written += bytes;
    }
    
    close(fd);
    printf("Arquivo %s criado: %ld bytes\n", filename, written);
}

void read_large_file(const char* filename) {
    printf("Lendo arquivo %s...\n", filename);
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir arquivo para leitura");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    size_t total_read = 0;
    ssize_t bytes;
    
    while ((bytes = read(fd, buffer, BUFFER_SIZE)) > 0) {
        total_read += bytes;
        // Simular algum processamento com os dados lidos
        for (ssize_t i = 0; i < bytes; i++) {
            buffer[i] = buffer[i] ^ 0x55; // Operação simples nos dados
        }
    }
    
    close(fd);
    printf("Arquivo %s lido: %ld bytes processados\n", filename, total_read);
}

void io_intensive_test() {
    printf("=== TESTE I/O-INTENSIVE ===\n");
    printf("PID: %d\n", getpid());
    
    char filenames[NUM_FILES][32];
    
    // Fase 1: Criação de arquivos grandes (escrita)
    printf("\n--- FASE 1: ESCRITA INTENSIVA ---\n");
    for (int i = 0; i < NUM_FILES; i++) {
        snprintf(filenames[i], sizeof(filenames[i]), "test_file_%d.dat", i);
        create_large_file(filenames[i], FILE_SIZE);
    }
    
    // Fase 2: Leitura intensiva
    printf("\n--- FASE 2: LEITURA INTENSIVA ---\n");
    for (int i = 0; i < NUM_FILES; i++) {
        read_large_file(filenames[i]);
    }
    
    // Fase 3: Leitura/escrita alternada
    printf("\n--- FASE 3: LEITURA/ESCRITA ALTERNADA ---\n");
    for (int round = 0; round < 3; round++) {
        printf("Round %d/3:\n", round + 1);
        
        for (int i = 0; i < NUM_FILES; i++) {
            read_large_file(filenames[i]);
            
            // Re-escrever parcialmente
            int fd = open(filenames[i], O_WRONLY);
            if (fd != -1) {
                char buffer[BUFFER_SIZE];
                memset(buffer, 'Y', BUFFER_SIZE);
                write(fd, buffer, BUFFER_SIZE);
                close(fd);
            }
        }
    }
    
    // Limpeza
    printf("\n--- LIMPEZA ---\n");
    for (int i = 0; i < NUM_FILES; i++) {
        if (unlink(filenames[i]) == 0) {
            printf("Arquivo %s removido\n", filenames[i]);
        }
    }
    
    printf("Teste I/O-intensive concluido!\n");
}

int main() {
    io_intensive_test();
    return 0;
}