#include "kernel/types.h"
#include "user/user.h"

#define MAXTOKEN 32
#define BUFSIZE 100

char buf[BUFSIZE];
int bufp = 0;

char* read_line(void);
int main(int argc, char *argv[]){
    char* b = read_line();
    printf("read:%s\n",b);
    exit(0);
}


char* read_line(void){
    char c;
    while(read(0,&c,sizeof c) != 0){
        if(c == '\n') break;
        if(c != ' ' || c != '\r'){
            buf[bufp++] = c;
        }
    }
    buf[bufp] = '\0';
    bufp = 0;
    return buf;
}
