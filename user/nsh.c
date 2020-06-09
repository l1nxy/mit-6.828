#include "kernel/types.h"
#include "user/user.h"

#define TOKENSIZE 32
#define MAXCMDSIZE 10

char token[TOKENSIZE];
char cmd[MAXCMDSIZE][TOKENSIZE];
int token_pos = 0;
int cmd_pos = 0;

int get_token();
int run_cmd(char *cmd, char **argv)
{
    if (fork() == 0)
    {
        int ret = exec(cmd, argv);
        if (ret == -1)
            fprintf(3, "exec %s error\n", cmd);
        exit(0);
    }
    else{
        wait(0);
        return 0;
    }

}
int main(int argc, char *argv[])
{
    int read_ret;
    while ((read_ret = get_token()))
    {
        strcpy(cmd[cmd_pos], token);
        cmd_pos++;
        if (read_ret == 1)
            break;
    }
    char *icmd[MAXCMDSIZE] = {0};
    for (int i = 0; i < cmd_pos; ++i)
    {
        ///TODO split command
        /*if (strcmp(cmd[i], "|") == 0 || strcmp(cmd[i], "<") == 0 || strcmp(cmd[i], ">"))*/
        /*{*/
            /*break;*/
        /*}*/
        icmd[i] = cmd[i];
    }
    run_cmd(cmd[0], icmd);
    exit(0);
}

int get_token()
{
    char c;
    while (read(0, &c, sizeof c) != 0)
    {
        if (c == ' ' || c == '\n')
            break;
        token[token_pos++] = c;
        if (token_pos == TOKENSIZE)
            break;
    }
    token[token_pos] = '\0';
    token_pos = 0;
    if (c == '\n')
        return 1;
    else
        return 2;
}
