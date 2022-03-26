#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_CHILDREN 3

int main(void)
{
    pid_t pid;

    int fd[2];
    FILE *f;

    int num_process;
    for (num_process = 0; num_process < MAX_CHILDREN; num_process++)
    {
        if (pipe(fd) == -1)
        {
            perror("pipe Failed");
            return 0;
        }

        pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0)
        {             // child code
            sleep(1); // for a nice output
            char buff[256];

            printf("Child %i (pid= %i)\n", num_process, getpid());
            close(fd[1]);

            while (read(fd[0], buff, sizeof(buff)) > 0)
            {
                printf("Read child = %s\n", buff);
            }
            close(fd[0]); // neccessary
            exit(0);
        }
        else
        { // parent
            printf("Im parent %i\n", getpid());
            close(fd[0]);

            int i;
            int str_len = 256;
            char str[str_len];
            f = fopen("input.txt", "r");

            while (!feof(f))
            {
                fgets(str, str_len, f);
                write(fd[1], str, strlen(str));
                printf("Parent send %s\n", str);
            }
            close(fd[1]); // neccessary
            wait(NULL);
        }
    }
    fclose(f);
    return 0;
}