#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREADS 2
#define ITERATIONS 1000000L  // REDUZIDO para teste rápido

void* cpu_intensive_task(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d iniciando...\n", thread_id);
    
    volatile double result = 0.0;
    for (long i = 0; i < ITERATIONS; i++) {
        result += sin(i * 0.001) * cos(i * 0.001);
    }
    
    printf("Thread %d concluída. Resultado: %f\n", thread_id, result);
    return NULL;
}

int main() {
    printf("=== TESTE CPU-INTENSIVE (RAPIDO) ===\n");
    printf("PID: %d\n", getpid());
    printf("Criando %d threads com %ld iteracoes cada...\n", NUM_THREADS, ITERATIONS);
    
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, cpu_intensive_task, &thread_ids[i]) != 0) {
            perror("Erro ao criar thread");
            return 1;
        }
        printf("Thread %d criada\n", i);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d finalizada\n", i);
    }
    
    printf("Teste CPU-intensive concluído!\n");
    return 0;
}