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

#endif