#include "userapp.h"

int main(int argc, char* argv[])
{
    /*
    The argument validation process.
    For every single period, we define our userapp to run around 90 msec, the user
    can just input 100 as the second argument to use this app.
    */
    if (argc != 4) {
        printf("Invalid arguments. \n usage: %s <period> <proc_time> <num_of_period>\n",argv[0]);
        printf(" The processing time for this application, aka, arg[2] is 100ms\n");
        return 1;
    }

    int i;
    
    /*
    The input parsing, str to int process.
    */
    int num_of_period = atoi(argv[3]);
    int period = atoi(argv[1]);
    int proc_time = atoi(argv[2]);

    /*
    For read content from /proc/mp2/status
    */
    char buf[BUFFERLEN];
    FILE *fd;
    char *tmp;
    char filename[] = MP2_PATH;
    int len;

    // Define variables to retrive the actual time elapse.
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
        do_job();
        gettimeofday(&end, NULL);

        print_result(t0,start,end,pid,period,proc_time);
        // echo yield to /proc/mp2/status
        yield_process(pid);
    }

    // echo unregister to finish scheduling of this process
    deregister_process(pid);

    return 0;
}
