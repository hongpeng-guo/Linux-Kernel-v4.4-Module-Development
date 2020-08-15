#include "userapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

int calculation(pid_t pid){
    unsigned input = (unsigned) pid;
    clock_t begin;
    begin = clock();
    while (clock()-begin < 5* CLOCKS_PER_SEC){
        input = (input*input)%1000;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    char command[100];
    pid_t child;
    int status = 0;

    for (int i=0; i<3; i++){
        child = fork();

        if (child == -1){
            printf ("Fork Fail!\n");
        }else if (child == 0){
            // printf ("I am the children, %u\n", getpid());
            sprintf(command, "echo %u > /proc/mp1/status", getpid());
            system(command);
            calculation(getpid());
            memset(command, 0, 100);
            sprintf(command, "cat /proc/mp1/status");
            system(command);
            exit(0);
        }else{
            wait(NULL);
        }
    }
	return 0;
}
