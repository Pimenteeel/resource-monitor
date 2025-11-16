#ifndef CGROUP_H
#define CGROUP_H

#include <stdbool.h>
#include <sys/types.h>

#define MAX_PATH 1024 

// Funções auxiliares (agora públicas para o main.c)
bool write_to_file(const char* filepath, const char* value);
unsigned long long read_from_file_ull(const char* filepath);
long read_from_file_long(const char* filepath);

// Funções principais do Control Group Manager
bool coletar_metricas_cgroup(const char* cgroup_path);
bool criar_configurar_cgroup(const char* controller, const char* name, double cpu_cores, long memoria_mb);
bool move_process_to_cgroup(const char* cgroup_path, int pid);
bool empty_and_delete_cgroup(const char* cgroup_path);
void gerar_relatorio_utilizacao(const char* cgroup_path);

#endif // CGROUP_H