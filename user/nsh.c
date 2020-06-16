#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

#define TOKENSIZE 32
#define MAXCMDSIZE 10

char token[TOKENSIZE];
char cmd[MAXCMDSIZE][TOKENSIZE];
int token_pos = 0;
int cmd_pos = 0;
char *icmd[MAXCMDSIZE] = {0};
/*
 echo hello there
 echo something > file.txt
 ls | grep READ
 grep lion < data.txt | wc > count
 echo echo hello | nsh
 find . b | xargs grep hello
 */
enum CMD_TYPE {
  NOMAL = 0,
  WRITE_REDIR,
};
int get_token();
int run_cmd(char *cmd, char **argv, int cmd_type) {
  if (cmd_type == NOMAL) {
    int ret = exec(cmd, argv);
    if (ret == -1) {
      fprintf(3, "exec %s error\n", cmd);
      exit(0);
    } else {
      wait(0);
      return 0;
    }
  } else if (cmd_type == WRITE_REDIR) {
    char *file_name;
    int i = 0;
    while (strcmp(icmd[i++], ">") != 0)
      ;
    file_name = icmd[i];
    argv[i-1] = 0;

    close(1);
    if (open(file_name, O_CREATE | O_WRONLY) < 0) {
      fprintf(2, "open %s fail\n", file_name);
      exit(1);
    }
    exec(cmd, argv);
    return 0;
  }
  return 0;
}

int redirection(int p) { return 0; }

int main(int argc, char *argv[]) {
  memset(cmd, 0, MAXCMDSIZE * TOKENSIZE);
  int read_ret;
  while ((read_ret = get_token())) {
    strcpy(cmd[cmd_pos++], token);
    if (read_ret == 1)
      break;
  }
  for (int i = 0; i < cmd_pos; ++i) {
    /// TODO split command
    /*if (strcmp(cmd[i], "|") == 0 || strcmp(cmd[i], "<") == 0 ||
     * strcmp(cmd[i],
     * ">"))*/
    /*{*/
    /*break;*/
    /*}*/
    icmd[i] = cmd[i];
  }
  for (int i = 0; i < MAXCMDSIZE; i++)
    printf("%s\n", cmd[i]);
  run_cmd(cmd[0], icmd, WRITE_REDIR);
  exit(0);
}

int get_token() {
  char c;
  while (read(0, &c, sizeof c) != 0) {
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
