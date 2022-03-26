#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{

    // int pipeFirstToSecond[2];

    int pipeFirstToSecond[2];

    char all[8192] = "";
    char all2[8192] = "";

    char inbuf[32];
    char outbuf[32];
    int nbytes;

    FILE *fp;

    // Create pipe - pipe_arr[0] is "reading end", pipe_arr[1] is "writing end"
    pipe(pipeFirstToSecond);

    if (fork() == 0) // child
    {
        // close reading end
        close(pipeFirstToSecond[0]);

        char f[64] = "/bin/ping -c 1";
        strcat(f, " google.com");
        printf(f);

        fp = popen(f, "r");
        if (fp == NULL)
        {
            printf("Failed to run command\n");
            exit(1);
        }

        /* Read the output a line at a time - output it. */
        while (fgets(inbuf, sizeof(inbuf), fp) != NULL)
        {
            strcat(all, inbuf);
        }

        write(pipeFirstToSecond[1], all, strlen(all) + 1);

        /* close */
        pclose(fp);

        // close writing end
        close(pipeFirstToSecond[1]);
    }
    else // parent - first
    {
        // close writing end
        close(pipeFirstToSecond[1]);

        while ((nbytes = read(pipeFirstToSecond[0], outbuf, strlen(outbuf) + 1)) > 0)
            strcat(all2, outbuf);

        printf("%s\n", all2);

        if (nbytes != 0)
            exit(2);

        printf("Finished reading\n");

        // close reading end
        close(pipeFirstToSecond[0]);
        wait(NULL);
    }

    return 0;
}

// Name or service not known or 100% packet loss

// ping: gooqgale.com: Name or service not known
// ?2 /mnt/c/Users/denis/Desktop % ping test.com -c 2                                                                                                                   23:33:37
// PING test.com (67.225.146.248) 56(84) bytes of data.

// --- test.com ping statistics ---
// 2 packets transmitted, 0 received, 100% packet loss, time 1004ms

// ?1 /mnt/c/Users/denis/Desktop % ping google.com -c 2                                                                                                                 23:34:09
// PING google.com (142.250.184.142) 56(84) bytes of data.
// 64 bytes from sof02s43-in-f14.1e100.net (142.250.184.142): icmp_seq=1 ttl=59 time=4.63 ms
// 64 bytes from sof02s43-in-f14.1e100.net (142.250.184.142): icmp_seq=2 ttl=59 time=6.27 ms

// --- google.com ping statistics ---
// 2 packets transmitted, 2 received, 0% packet loss, time 1001ms
// rtt min/avg/max/mdev = 4.633/5.453/6.273/0.820 ms