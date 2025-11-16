#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CHUNK_SIZE (100 * 1024 * 1024) // 100MB
#define NUM_CHUNKS 10
#define TOTAL_MEMORY (CHUNK_SIZE * NUM_CHUNKS) // ~1GB

void memory_intensive_test() {
    printf("=== TESTE MEMORY-INTENSIVE ===\n");
    printf("PID: %d\n", getpid());
    printf("Alocando %d chunks de %d MB cada (total: %.1f GB)...\n", 
           NUM_CHUNKS, CHUNK_SIZE / (1024 * 1024), 
           (double)TOTAL_MEMORY / (1024 * 1024 * 1024));
    
    char* memory_chunks[NUM_CHUNKS];
    size_t allocated = 0;
    
    // Alocar memória gradualmente
    for (int i = 0; i < NUM_CHUNKS; i++) {
        printf("Alocando chunk %d/%d (%d MB)...\n", i + 1, NUM_CHUNKS, CHUNK_SIZE / (1024 * 1024));
        
        memory_chunks[i] = malloc(CHUNK_SIZE);
        if (memory_chunks[i] == NULL) {
            printf("Falha na alocacao do chunk %d. Total alocado: %.1f MB\n", 
                   i + 1, (double)allocated / (1024 * 1024));
            break;
        }
        
        // Preencher a memória com dados para garantir uso real
        memset(memory_chunks[i], 0xAA, CHUNK_SIZE);
        allocated += CHUNK_SIZE;
        
        printf("Chunk %d alocado e preenchido. Total: %.1f MB\n", 
               i + 1, (double)allocated / (1024 * 1024));
        
        sleep(2); // Intervalo entre alocações
    }
    
    printf("Memoria alocada com sucesso: %.1f MB\n", (double)allocated / (1024 * 1024));
    printf("Mantendo memoria alocada por 30 segundos...\n");
    
    // Manter a memória alocada por um tempo
    for (int sec = 0; sec < 30; sec++) {
        // Acessar memória periodicamente para evitar swap
        for (int i = 0; i < NUM_CHUNKS && memory_chunks[i] != NULL; i++) {
            if (sec % 5 == 0) {
                // Acesso de leitura
                volatile char access = memory_chunks[i][0];
                (void)access; // Evitar warning
            }
        }
        sleep(1);
    }
    
    // Liberar memória
    printf("Liberando memoria...\n");
    for (int i = 0; i < NUM_CHUNKS; i++) {
        if (memory_chunks[i] != NULL) {
            free(memory_chunks[i]);
        }
    }
    
    printf("Teste memory-intensive concluido!\n");
}

int main() {
    memory_intensive_test();
    return 0;
}