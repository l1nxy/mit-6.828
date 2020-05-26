#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

char c;
int i = 0;
char buf[256];
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "xargs command need more args\n");
        exit(1);
    }
    int n;
    while ((n = read(0, &c, 1)) > 0)
    {
        if (n == 1 && c != '\n')
        {
            buf[i++] = c;
            continue;
        }
        if (c == '\n')
        {
            buf[i] = '\0';
            i = 0;
            argv[argc] = buf;
            argv[argc+1] = 0;
            if (fork() == 0)
            {
                exec(argv[1], argv + 1);
                exit(1);
            }
            else
            {
                wait(0);
            }
            buf[i] = '\0';
        }
    }
    exit(1);
}
