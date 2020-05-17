#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>

int main()
{
    pid_t p1;
    pid_t p2;
    prctl(PR_SET_PDEATHSIG, SIGINT);
    p1 = fork();
    if (p1 == 0)
    {
        printf("__y__\n");
        printf("p1 pid: %d\n\n", getpid());
        return 0;
    }
    p2 = fork();
    if (p2 == 0)
    {
        printf("__z__\n");
        printf("p2 pid: %d\n\n", getpid());
        return 0;
    }
    printf("main pid: %d\n\n", getpid());
    printf("__x__\n");
    return 0;
}
