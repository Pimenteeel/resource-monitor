#ifndef NAMESPACE_H
#define NAMESPACE_H

typedef struct
{
    long cgroup;
    long ipc;
    long mnt;
    long net;
    long pid;
    long time;
    long user;
    long uts;
} ProcessNamespaces;

int namespaces_por_pid(int pid, ProcessNamespaces *ns);

void comparar_namespaces(int pid1, int pid2);

void mapear_todos_processos();

void namespace_overhead(long iteracao);

#endif