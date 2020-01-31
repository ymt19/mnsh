#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char cmd[1024];
    int status;
    pid_t cpid;

    for (;;) {
        fprintf(stderr, "$ ");
        scanf("%s", cmd);

        if ((cpid = fork()) == -1) {
            // fork error
            perror("fork");
            exit(1);
        } else if (cpid == 0) {
            // child process
            execlp(cmd, cmd, (char*)0);
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