#include "mnsh.h"

#define MAXARGV 100
#define MAXLINE 4096

int main(void) {
    char cmd[MAXLINE];
    char *av[MAXARGV];
    int status, ac;
    pid_t cpid;

    for (;;) {
        // プロンプトの表示, 文字列受け取り
        fprintf(stderr, "$ ");
        fgets(cmd, MAXLINE, stdin);
        
        // トークナイズする
        if ((token = tokenize(cmd)) == NULL) {
            continue;
        }

        // パースする(二分木を作る)
        Node *node = expr();
        printf("%s\n", node->left->cmd[0]);
        printf("%s\n", node->right->cmd[0]);

        if ((cpid = fork()) == -1) {
            // fork error
            perror("fork");
            exit(1);
        } else if (cpid == 0) {
            // child process
            chexec(node);
            perror(cmd);
            exit(1);
        }

        if (wait(&status) == (pid_t)-1) {
            perror("wait");
            exit(1);
        }
    }
    return 0;
}