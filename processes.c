#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
int main()
{
    // lets create a family!
    pid_t grandpa_pid = getpid();
    pid_t child_pid;
    if ((child_pid = fork()) == 0)
    {
        pid_t parent_pid = getpid();
        pid_t child_child_pid;
        if ((child_child_pid = fork()) == 0)
        {
            usleep(200);
            printf("I am child and I will write third. My pid is %d, %d is my parent and %d is my parents parent, so my grandpa.", getpid(), parent_pid, grandpa_pid);
        }
        else
        {
            usleep(100);
            // parent_pid = getpid()
            printf("I am parent and I will write second. My pid is %d, %d is my child and %d is my parent.", getpid(), child_child_pid, grandpa_pid);
        }
    }
    else
    {
        // grandpa_pid == getpid()
        printf("I am grandpa and I will write first. My pid is %d and %d is my child.", getpid(), child_pid);
    }
    printf("\n");
}