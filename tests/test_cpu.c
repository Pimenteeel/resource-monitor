#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREADS 4
#define ITERATIONS 20000000000L

void* cpu_intensive_task(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d iniciando tarefa CPU-intensive...\n", thread_id);
    
    volatile double result = 0.0;
    for (long i = 0; i < ITERATIONS; i++) {
        // Cálculos matemáticos pesados
        result += sin(i * 0.001) * cos(i * 0.001) + 
                 tan(i * 0.0001) * log(i + 1.0) +
                 sqrt(i * 0.01) * pow(2.71828, i * 0.00001);
    }
    
    printf("Thread %d concluída. Resultado: %f\n", thread_id, result);
    return NULL;
}

int main() {
    printf("=== TESTE CPU-INTENSIVE ===\n");
    printf("PID: %d\n", getpid());
    printf("Criando %d threads com %ld iteracoes cada...\n", NUM_THREADS, ITERATIONS);
    
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    // Criar múltiplas threads para maximizar uso da CPU
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, cpu_intensive_task, &thread_ids[i]) != 0) {
            perror("Erro ao criar thread");
            return 1;
        }
    }
    
    // Aguardar todas as threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Teste CPU-intensive concluído!\n");
    return 0;
}