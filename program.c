#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int pipeSecondChildrenToFirst[2];

void startChild(char *ip)
{
    pid_t pid;
    int i, stat;

    // printf("Testing %s\n", ip);

    FILE *fp;

    char commandOutputBytes[1024];
    char commandOutput[8192];

    if (pid = fork() == 0)
    {
        // redirect the stderr to stdout
        char command[64] = "/bin/ping 2>&1 -c 1 -w 1 ";
        strcat(command, ip);

        fp = popen(command, "r");

        if (fp == NULL)
        {
            printf("Failed to run command\n");
            exit(1);
        }

        /* Read the output a line at a time - output it. */
        while (fgets(commandOutputBytes, sizeof(commandOutputBytes), fp) != NULL)
        {
            strcat(commandOutput, commandOutputBytes);
        }

        if (strstr(commandOutput, "Name or service not known") != NULL || strstr(commandOutput, "100% packet loss") != NULL)
        {
            write(pipeSecondChildrenToFirst[1], ip, strlen(ip) + 1);
        }
        // printf("IP %s - BEFORE OUT", ip);
        pclose(fp);
        // printf("IP %s - 222 OUT", ip);
        // // exit(0);
        // printf("IP %s - AFTER OUT", ip);
        // printf("%s OUT\n", ip);
    }
    // else
    // {
    //     wait(NULL);
    // }
}

void *myThreadFun(void *var)
{
    char buff[256];

    while (read(pipeSecondChildrenToFirst[0], buff, sizeof(buff)) > 0)
    {
        printf("Could not reach %s\n", buff);
    }
}

int main(void)
{

    int pipeFirstToSecond[2];

    FILE *ipsFile;

    // create pipes
    pipe(pipeFirstToSecond);
    pipe(pipeSecondChildrenToFirst);

    if (fork() == 0) // child
    {
        char ip[128];

        while (read(pipeFirstToSecond[0], ip, sizeof(ip)) > 0)
        {
            // printf("Child got %s", ip);
            startChild(ip);
        }

        printf("MAYBE HERE?");

        // close reading end
        close(pipeFirstToSecond[0]);
        printf("MAYBE HERE?1");
        wait(NULL);
        printf("MAYBE HERE?2");
        // close writing end
        close(pipeSecondChildrenToFirst[1]);
        printf("MAYBE HERE?3");
        exit(0);
        printf("MAYBE HERE?4");
    }
    else // parent - first
    {
        ipsFile = fopen("ips.dat", "r");
        int str_len = 256;
        char str[str_len];

        pthread_t tid;
        pthread_create(&tid, NULL, &myThreadFun, NULL);
        pthread_detach(tid);

        // send the ip to the child
        while (!feof(ipsFile))
        {
            fgets(str, str_len, ipsFile);
            write(pipeFirstToSecond[1], str, strlen(str) + 1);
            sleep(5);
        }
        // pthread_exit(&tid);
        printf("S\n");
        // close writing end
        close(pipeFirstToSecond[1]);
        printf("F\n");

        close(pipeSecondChildrenToFirst[0]);

        printf("G\n");

        wait(NULL);
        printf("K\n");
    }

    fclose(ipsFile);
    return 0;
}