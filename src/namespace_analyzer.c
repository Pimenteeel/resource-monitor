#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "namespace.h"

int namespaces_por_pid(int pid, ProcessNamespaces *ns){
    char proc_path[512];
    struct stat buffer; // lista vazia para guardar informações dos status (structure of status) de um arquivo

    // lógica para o mnt
    sprintf(proc_path, "/proc/%d/ns/mnt", pid);
    if (stat(proc_path, &buffer) == -1){
        return -1;
    }
    ns->mnt = (long)buffer.st_ino; //st_ino é o campo que armazena o número do inode (id do ns). o tipo original é ino_t

    // lógica para o uts
    sprintf(proc_path, "/proc/%d/ns/uts", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> uts = 0;
    }
    else{
        ns -> uts = (long)buffer.st_ino;
    }

    // lógica para o ipc
    sprintf(proc_path, "/proc/%d/ns/ipc", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> ipc = 0;
    }
    else{
        ns -> ipc = (long)buffer.st_ino;
    }

    // lógica para o net
    sprintf(proc_path, "/proc/%d/ns/net", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> net = 0;
    }
    else{
        ns -> net = (long)buffer.st_ino;
    }

    // lógica para o pid
    sprintf(proc_path, "/proc/%d/ns/pid", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> pid = 0;
    }
    else{
        ns -> pid = (long)buffer.st_ino;
    }

    // lógica para user
    sprintf(proc_path, "/proc/%d/ns/user", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> user = 0;
    }
    else{
        ns -> user = (long)buffer.st_ino;
    }

    // lógica para cgroup
    sprintf(proc_path, "/proc/%d/ns/cgroup", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> cgroup = 0;
    }
    else{
        ns -> cgroup = (long)buffer.st_ino;
    }

    // lógica para time
    sprintf(proc_path, "/proc/%d/ns/time", pid);
    if (stat(proc_path, &buffer) == -1){
        ns -> time = 0;
    }
    else{
        ns -> time = (long)buffer.st_ino;
    }

    return 0;
}