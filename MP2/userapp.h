#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define MP2_PATH "/proc/mp2/status"
#define BUFFERLEN     5000

// echo register command to /proc/mp2/status.
void register_process(unsigned int pid, unsigned int period, unsigned int proc_time)
{
    char *command = NULL;
    command = malloc(100);
    memset(command, '\0', 100);
    sprintf(command, "echo \"R, %u, %u, %u\" > %s", pid, period, proc_time, MP2_PATH);
    system(command);
    free(command);
}

//echo yeild command to status file when current period work done.
void yield_process(unsigned int pid)
{
    char *command = NULL;
    command = malloc(100);
    memset(command, '\0', 100);
    sprintf(command, "echo \"Y, %u\" > %s", pid, MP2_PATH);
    system(command);
    free(command);
}

// echo deregister info to finish this process scheduling.
void deregister_process(unsigned int pid)
{
    char *command = NULL;
    command = malloc(100);
    memset(command, '\0', 100);
    sprintf(command, "echo \"D, %u\" > %s", pid, MP2_PATH);
    system(command);
    free(command);
}

/*
A random modular factorial calculation. Using time components
to ensure the execuation time is close but smaller than 100 msec.
*/
void do_job() {
    int itr, result=1;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    gettimeofday(&end, NULL);
    while((end.tv_sec*1000.0+end.tv_usec/1000.0) - (start.tv_sec*1000.0 + start.tv_usec/1000.0) < 90){
        result = (result* (result+1))%500;
        gettimeofday(&end, NULL);
    }
    return;
}

/*
Output format of userapp. It will print scheduling details of this process after evey
execuation period.
*/
void print_result(struct timeval initial,struct timeval start,struct timeval end,unsigned int pid,int period,int proc_time){
    double actual_proc_time = (end.tv_sec*1000.0+end.tv_usec /1000.0)-(start.tv_sec*1000+start.tv_usec/1000.0);
    double start_time = (start.tv_sec*1000.0+start.tv_usec/1000.0)-(initial.tv_sec*1000.0+initial.tv_usec/1000.0);
    double end_time = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(initial.tv_sec*1000.0+initial.tv_usec/1000.0);
    printf("%u\t%d\t%d\t%f\t%f\t%f\n",pid,period,proc_time,actual_proc_time,start_time,end_time);
}
