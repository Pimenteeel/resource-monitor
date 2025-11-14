#ifndef NAMESPACE_H
#define NAMESPACE_H
#include <sys/types.h>

typedef struct
{
    ino_t cgroup;
    ino_t ipc;
    ino_t mnt;
    ino_t net;
    ino_t pid;
    ino_t time;
    ino_t user;
    ino_t uts;
} ProcessNamespaces;

int namespaces_por_pid(pid_t pid, ProcessNamespaces *ns);

#endif