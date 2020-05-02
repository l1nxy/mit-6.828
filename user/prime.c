#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int prime_aux(int fd)
{
    int pfd[2],wfd,num;
    int i;
    int pid;
top:
    pid = getpid();
    printf("process:%d\n",pid);
    if(read(fd,&num,sizeof num) != 4)
    {
        printf("read error\n");
        exit(0);
    }
    printf("prime:%d\n",num);

    pipe(pfd);
    int id;
    id = fork();
    if(id < 0)
    {
        printf("fd :%d\n",fd);
        printf("fork error:%d\n",id);
        exit(0);
    }
    if(id == 0)
    {
        close(fd);
        close(pfd[1]);
        fd = pfd[0];
        goto top;
    }
    close(pfd[0]);
    wfd = pfd[1];
    while(1)
    {
        read(fd,&i,sizeof i);
        if(i % num)
        {
            write(wfd,&i , sizeof i);
        }
    }

}
int main()
{
    int pid = getpid();
    printf("%d\n",pid);
    int p[2];
    int id;
    if(pipe(p) < 0)
    {
        printf("pipe error\n");
        exit(0);
    }
    id = fork();
    if(id < 0)
    {
        printf("fork error :%d\n", id);
        exit(0);
    }
    if(id == 0)
    {
        close(p[1]);
        prime_aux(p[0]);
    }

    close(p[0]);
    for(int i = 2; i< 35; i++)
    {
        if(write(p[1],&i,sizeof i) != 4)
        {
            printf("write error\n");
            exit(0);
        }
    }
    return 0;
}
