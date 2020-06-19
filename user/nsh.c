#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

#define TOKENSIZE 16
#define MAXCMDSIZE 50

char token[TOKENSIZE];
char cmd[MAXCMDSIZE][TOKENSIZE];
int token_pos = 0;
int cmd_pos = 0;
char *leftcmd[MAXCMDSIZE] = {0};
char *rightcmd[MAXCMDSIZE] = {0};
/*
 echo hello there
 echo something > file.txt
 ls | grep READ
 grep lion < data.txt | wc > count
 echo echo hello | nsh
 find . b | xargs grep hello
 */
enum CMD_TYPE {
  NOMAL_CMD = 0,
  WRITE_REDIR_CMD,
  READ_REDIR_CMD,
  PIPE_CMD,
};
int get_token();
int proxy_cmd(char **icmd);
int fork1(void);

int run_cmd_nomal(char *cmd, char **argv) {
  exec(cmd, argv);
  // fprintf(2,"run ret:%d\n",ret);
  fprintf(2, "exec %s error\n", cmd);
  return -1;
}
int run_cmd_redir(char *icmd, char **argv, int redir_type) {
  char *read_file_name;
  char *write_file_name;
  int read_type = 0;
  int write_type = 0;
  int i = 0;
  for (int index = 0;  index < cmd_pos; ++index) {
    if (strcmp(argv[index], "<") == 0) {
      read_type = 1;
    }
    if (strcmp(argv[index], ">") == 0) {
      write_type = 1;
    }
  }
  if (write_type == 1 && read_type == 0) {
    while (strcmp(argv[i++], ">") != 0)
      ;
    write_file_name = argv[i];
    argv[i - 1] = 0;
    close(1);
    if (open(write_file_name, O_CREATE | O_WRONLY) < 0) {
      fprintf(2, "open %s fail\n", write_file_name);
      return -1;
    }
  } else if (read_type == 1 && write_type == 0) {
    while (strcmp(argv[i++], "<") != 0)
      ;
    read_file_name = argv[i];
    argv[i - 1] = 0;
    close(0);
    if (open(read_file_name, O_RDONLY) < 0) {
      fprintf(2, "open %s fail\n", read_file_name);
      return -1;
    }
  } else if (read_type == 1 && write_type == 1) {
    while (strcmp(argv[i++], "<") != 0)
      ;

    read_file_name = argv[i];
    int need_set_zero = i-1;
    close(0);
    if (open(read_file_name, O_RDONLY) < 0) {
      fprintf(2, "open %s fail\n", read_file_name);
      return -1;
    }

    while (strcmp(argv[i++], ">") != 0)
      ;
    write_file_name = argv[i];
    argv[need_set_zero] = 0;
    close(1);
    if (open(write_file_name, O_CREATE | O_WRONLY) < 0) {
      fprintf(2, "open %s fail\n", write_file_name);
      return -1;
    }
  }
  exec(icmd, argv);
  fprintf(2,"run %s failed\n",argv[0]);
  wait(0);
  return 0;
}

int run_cmd_pipe(char *cmd1, char **argv1, char *cmd2, char **argv2) {
  int p[2];
  pipe(p);
  if (fork1() == 0) {
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    proxy_cmd(argv1);
  }
  if (fork1() == 0) {
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    proxy_cmd(argv2);
  }
  close(p[0]);
  close(p[1]);
  wait(0);
  wait(0);
  return 0;
}

int run_cmd(char *cmd, char **argv, int cmd_type) {
  if (cmd_type == NOMAL_CMD) {
    run_cmd_nomal(cmd, argv);
  } else if (cmd_type == WRITE_REDIR_CMD) {
    run_cmd_redir(cmd, argv, 1);
  } else if (cmd_type == READ_REDIR_CMD) {
    run_cmd_redir(cmd, argv, 0);
  }
  return 0;
}

int get_cmd_size(char **cmd) {
  int size = 0;
  while (cmd[size++] != 0)
    ;
  return size;
}
int proxy_cmd(char **icmd) {
  int cmd_size = get_cmd_size(icmd);
  for (int i = 0; i < cmd_size; ++i) {
    if (strcmp(">", icmd[i]) == 0) {
      run_cmd_redir(icmd[0], icmd, WRITE_REDIR_CMD);
      return 0;
    } else if (strcmp("<", icmd[i]) == 0) {
      run_cmd_redir(icmd[0], icmd, READ_REDIR_CMD);
      return 0;
    }
  }
  run_cmd_nomal(icmd[0], icmd);
  return 0;
}

int parse_cmd() {
  int i = 0;
  for (; i < cmd_pos; ++i) {
    if (strcmp("|", cmd[i]) == 0) {
      int k = 0;
      for (int j = i + 1; j < cmd_pos; j++, k++) {
        rightcmd[k] = cmd[j];
      }
      leftcmd[i] = 0;
      rightcmd[k] = 0;
      //fprintf(2,"left:%s %s %s\n",leftcmd[0],leftcmd[1],leftcmd[2]);
      //fprintf(2,"rightcmd:%s %s\n",rightcmd[0],rightcmd[1]);
      run_cmd_pipe(cmd[0], leftcmd, cmd[i + 1], rightcmd);
      return 0;
    } else {
      leftcmd[i] = cmd[i];
    }
  }
  leftcmd[i] = 0;
  proxy_cmd(leftcmd);
  return 0;
}

int get_token() {
  char c = 0;
  int read_ret;
  while ((read_ret = read(0, &c, sizeof c)) != 0) {
    if (c == ' ' || c == '\n')
      break;
    token[token_pos++] = c;
    if (token_pos == TOKENSIZE)
      break;
  }
  token[token_pos] = '\0';
  //fprintf(2,"get_token:%s\n",token);
  token_pos = 0;
  if (c == 0)
    return 0;
  if (c == '\n')
    return 1;
  else
    return 2;
}
void panic(char *s)
{
    //fprintf(2, "%s\n", s);
    exit(1);
}

int fork1(void)
{
    int pid;

    pid = fork();
    if (pid == -1)
        panic("fork");
    return pid;
}

int main(int argc, char *argv[]) {
  fprintf(2, "@ ");
  int read_ret;
  while (1) {
    read_ret = get_token();
    if (read_ret == 0)
      exit(0);
    strcpy(cmd[cmd_pos++], token);
    if (read_ret == 1) {
       // int fork_ret = 0;
        if(fork1() == 0)
            parse_cmd();
      wait(0);
      cmd_pos = 0;
      memset(cmd, 0, MAXCMDSIZE * TOKENSIZE);
      fprintf(2, "@ ");
    }
  }
  return 0;
}
