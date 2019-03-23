#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_FACTOR 999999
#define MOD_FACTOR 20
#define MP2_PATH "/proc/mp2/status"
#define BUFFERLEN     5000


void register_process(unsigned int pid, unsigned int period, unsigned int proc_time)
{
    char *command = NULL;
    command = malloc(100);
    memset(command, '\0', 100);
    sprintf(command, "echo \"R, %u, %u, %u\" > %s", pid, period, proc_time, MP2_PATH);
    system(command);
    free(command);
}

void yield_process(unsigned int pid)
{
    char *command = NULL;
    command = malloc(100);
    memset(command, '\0', 100);
    sprintf(command, "echo \"Y, %u\" > %s", pid, MP2_PATH);
    system(command);
    free(command);
}

void unregister_process(unsigned int pid)
{
    char *command = NULL;
    command = malloc(100);
    memset(command, '\0', 100);
    sprintf(command, "echo \"D, %u\" > %s", pid, MP2_PATH);
    system(command);
    free(command);
}

void do_job(int times) {
    int time, i, j, res = 1;
    for(time=0;time<times;time++)
    {
        for (i=0; i<MAX_FACTOR; i++) {
            for (j=0; j < i % MOD_FACTOR; j++) {
                res *= j;
            }
        }
    }
}

void print_result(struct timeval initial,struct timeval start,struct timeval end,unsigned int pid,int period,int proc_time){
    double actual_proc_time = (end.tv_sec*1000.0+end.tv_usec /1000.0)-(start.tv_sec*1000+start.tv_usec/1000.0);
    double start_time = (start.tv_sec*1000.0+start.tv_usec/1000.0)-(initial.tv_sec*1000.0+initial.tv_usec/1000.0);
    double end_time = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(initial.tv_sec*1000.0+initial.tv_usec/1000.0);
    printf("%u\t%d\t%d\t%f\t%f\t%f\n",pid,period,proc_time,actual_proc_time,start_time,end_time);
}
