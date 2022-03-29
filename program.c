#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int pipeChildrenToParent[2];
int pipeParentToChild[2];

void parentProcess();
void *printErrorThread();
void childProcess();
void startChild(char *ip);

int main(void)
{

    // create pipes
    if (pipe(pipeParentToChild) == -1 || pipe(pipeChildrenToParent) == -1)
    {
        printf("Error: Creating pipe failed.\n");
        exit(0);
    }

    if (fork() == 0) // child
    {
        childProcess();
    }
    else // parent
    {
        parentProcess();
    }

    return 0;
}

void parentProcess()
{
    // close reading end
    close(pipeParentToChild[0]);

    // close writing end
    close(pipeChildrenToParent[1]);

    FILE *ipsFile;

    ipsFile = fopen("ips.dat", "r");
    if (ipsFile == NULL)
    {
        perror("Failed");
        exit(1);
    }

    int currentIP_len = 256;
    char currentIP[currentIP_len];

    pthread_t tid;
    pthread_create(&tid, NULL, &printErrorThread, NULL);
    pthread_detach(tid);

    // send the ips to the child
    while (!feof(ipsFile))
    {
        fgets(currentIP, currentIP_len, ipsFile);
        write(pipeParentToChild[1], currentIP, strlen(currentIP) + 1);
        sleep(2);
    }

    fclose(ipsFile);

    // close writing end
    close(pipeParentToChild[1]);

    wait(NULL);
}

void *printErrorThread()
{
    char returnedIP[256];

    while (read(pipeChildrenToParent[0], returnedIP, sizeof(returnedIP)) > 0)
    {
        printf("Could not reach %s\n", returnedIP);
    }

    // close reading end
    close(pipeChildrenToParent[0]);

    return NULL;
}

void childProcess()
{
    // close writing end
    close(pipeParentToChild[1]);

    // close reading end
    close(pipeChildrenToParent[0]);

    char ip[128];

    while (read(pipeParentToChild[0], ip, sizeof(ip)) > 0)
    {
        startChild(ip);
    }

    // close reading end
    close(pipeParentToChild[0]);

    wait(NULL);

    // close writing end
    close(pipeChildrenToParent[1]);
}
void startChild(char *ip)
{
    FILE *fp;

    char commandOutputLine[1024];
    char commandOutput[8192];

    if (fork() == 0)
    {
        // redirect the stderr to stdout with timeout set to 1 sec
        char command[64] = "/bin/ping 2>&1 -c 1 -w 1 ";
        strcat(command, ip);

        fp = popen(command, "r");

        if (fp == NULL)
        {
            printf("Failed to run command\n");
            exit(1);
        }

        while (fgets(commandOutputLine, sizeof(commandOutputLine), fp) != NULL)
        {
            strcat(commandOutput, commandOutputLine);
        }

        if (strstr(commandOutput, "Name or service not known") != NULL || strstr(commandOutput, "100% packet loss") != NULL)
        {
            write(pipeChildrenToParent[1], ip, strlen(ip) + 1);
        }
        pclose(fp);
        exit(0);
    }
}