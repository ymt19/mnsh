#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "mnsh.h"

#define MAXARGV 100
#define MAXLINE 4096

int main(void) {
    char cmd[MAXLINE];
    char *av[MAXARGV];
    int status, ac;
    pid_t cpid;

    for (;;) {
        fprintf(stderr, "$ ");
        fgets(cmd, MAXLINE, stdin);
        
        if ((ac = tokenize(cmd, av, MAXARGV)) > MAXARGV) {
            fprintf(stderr, "too many arguments\n");
            continue;
        }
        if (ac == 1) continue;

        if ((cpid = fork()) == -1) {
            // fork error
            perror("fork");
            exit(1);
        } else if (cpid == 0) {
            // child process
            execvp(av[0], av);
            perror(cmd);
            exit(1);
        }

        if (wait(&status) == (pid_t)-1) {
            perror("wait");
            exit(1);
        }
    }
    exit(0);
}