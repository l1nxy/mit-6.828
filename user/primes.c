#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime_aux(int fd)
{
    int pfd[2], num;
    int i;
top:
    while (read(fd, &num, sizeof num) != 4)
        ;
    printf("prime %d\n", num);

    pipe(pfd);
    int id;
    id = fork();
    if (id < 0)
    {
        exit(1);
    }
    if (id == 0)
    {
        close(fd);
        close(pfd[1]);
        fd = pfd[0];
        goto top;
    }
    close(pfd[0]);
    while (1)
    {
        if(read(fd, &i, sizeof i) != sizeof i)
            exit(0);
        if (i % num)
        {
            if(write(pfd[1], &i, sizeof i) !=sizeof i)
                exit(0);
        }
    }
}
int main(void)
{
    int p[2];
    pipe(p);
    if (fork() == 0)
    {
        close(p[0]);
        for (int i = 2; i < 35; i++)
        {
            if (write(p[1], &i, sizeof i) != 4)
            {
                printf("write error\n");
                exit(0);
            }
        }
    }
    close(p[1]);
    prime_aux(p[0]);

    exit(0);
}
