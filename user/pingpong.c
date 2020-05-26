#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int parent_fd[2];
int child_fd[2];
char buf[256];
int main()
{
    pipe(parent_fd);
    pipe(child_fd);
    if(fork() == 0)
    {
        close(0);
        close(parent_fd[1]);
        int n = read(parent_fd[0],buf,sizeof buf);
        buf[n]='\0';
        int pid = getpid();
        printf("%d: received %s\n",pid,buf);
        close(child_fd[0]);
        write(child_fd[1],"pong",5);
        exit(0);
    }
    else
    {
        close(parent_fd[0]);
        write(parent_fd[1],"ping",5);

        int n = read(child_fd[0],buf,sizeof buf);
        buf[n] = '\0';
        int pid = getpid();
        printf("%d: received %s\n",pid,buf);
        exit(0);
    }
}
