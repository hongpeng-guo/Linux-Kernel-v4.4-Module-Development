/*
    reference:
        https://github.com/calvinmwhu/CS423/blob/master/mp2/mp2_user_app.c
*/

#include "userapp.h"

/*
    argv[1] -> period
    argv[2] -> computation time
    argv[3] -> number of period
*/

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Invalid arguments. \n usage: %s <period> <proc_time> <num_of_period>\n",argv[0]);
        return 1;
    }

    int i;

    int num_of_period = atoi(argv[3]);
    int period = atoi(argv[1]);
    int proc_time = atoi(argv[2]);

    char buf[BUFFERLEN];
    FILE *fd;
    char *tmp;
    char filename[] = MP2_PATH;
    int len;

    // when fact_iteration = 10
    // the computation time is about 200ms for my computer
    int fact_iteration = 10;
    unsigned int pid = getpid();
    struct timeval t0, start, end;

    printf("period: %d, computation time: %d, number of period: %d\n",period,proc_time,num_of_period);    
    
    // register process by echo to /proc/mp2/status
    register_process(pid, period, proc_time);

    fd = fopen(filename, "r");
    if(fd==NULL){
        printf("Fail to open %s",MP2_PATH);
    }
    len = fread(buf, sizeof(char), BUFFERLEN - 1, fd);
    buf[len] = '\0';
    fclose(fd);
    printf("show registed pids\n");
    printf("%s\n",buf);

    // record when the task start
    gettimeofday(&t0, NULL);

    yield_process(pid);
    printf("pid\tperiod\tproc_t\tactu_proc_t\tstart_time\tend_time\n");
    for (i = 0; i < num_of_period; i ++) {
        gettimeofday(&start, NULL);
        do_job(fact_iteration);
        gettimeofday(&end, NULL);

        print_result(t0,start,end,pid,period,proc_time);
        // echo yield to /proc/mp2/status
        yield_process(pid);
    }

    unregister_process(pid);

    return 0;
}
