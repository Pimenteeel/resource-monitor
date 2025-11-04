#include <stdio.h>
#include <stdlib.h>

void monitor_process(int pid){
    char proc_path[256];
    FILE *fp;

    sprintf(proc_path, "/proc/%d/status", pid);
}