#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <time.h>
#include <sys/mman.h>
#include <memory.h>

struct mt
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexattr;
};

int main()
{
    pid_t p1;
    pid_t p2;
    pid_t p3;
    int PublicPipe[2];
    char wbuffer[100];
    char rbuffer[100];
    prctl(PR_SET_PDEATHSIG, SIGINT);
    if (pipe(PublicPipe) == -1)
    {
        printf("Pipe error ! \n");
        exit(1);
    }
    struct mt *my_mutex;
    my_mutex = (mt *)mmap(NULL, sizeof(*my_mutex), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    memset(my_mutex, 0, sizeof(*my_mutex));
    pthread_mutexattr_init(&my_mutex->mutexattr);
    pthread_mutexattr_setpshared(&my_mutex->mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&my_mutex->mutex, &my_mutex->mutexattr);
    p1 = fork();
    if (p1 == 0)
    {
        for (;;)
        {
            printf("proc_1 pid: %d\n", getpid());
            pthread_mutex_lock(&my_mutex->mutex);
            printf("proc_1 sending\n");
            fflush(stdout);
            sprintf(wbuffer, "I'm proc_1");
            write(PublicPipe[1], wbuffer, sizeof(wbuffer));
            usleep(2000);
            printf("proc_1 send done.\n");
            fflush(stdout);
            pthread_mutex_unlock(&my_mutex->mutex);
        }
        return 0;
    }
    p2 = fork();
    if (p2 == 0)
    {
        for (;;)
        {
            printf("proc_2 pid: %d\n", getpid());
            pthread_mutex_lock(&my_mutex->mutex);
            printf("proc_2 sending\n");
            fflush(stdout);
            sprintf(wbuffer, "I'm proc_2");
            write(PublicPipe[1], wbuffer, sizeof(wbuffer));
            usleep(2000);
            printf("proc_2 send done.\n");
            fflush(stdout);
            pthread_mutex_unlock(&my_mutex->mutex);
        }
        return 0;
    }
    p3 = fork();
    if (p3 == 0)
    {
        for (;;)
        {
            printf("proc_3 pid: %d\n", getpid());
            pthread_mutex_lock(&my_mutex->mutex);
            printf("proc_3 sending\n");
            fflush(stdout);
            sprintf(wbuffer, "I'm proc_3");
            write(PublicPipe[1], wbuffer, sizeof(wbuffer));
            usleep(2000);
            printf("proc_3 send done.\n");
            fflush(stdout);
            pthread_mutex_unlock(&my_mutex->mutex);
        }
        return 0;
    }
    while (1)
    {
        if (read(PublicPipe[0], rbuffer, sizeof(wbuffer)) == -1)
        {
            printf("read error !\n");
            exit(1);
        }
        printf("%s\n", rbuffer);
    }
    return 0;
}