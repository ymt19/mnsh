#include "mnsh.h"

#define MAXLINE 4096

void handler(int sig) {
    fprintf(stderr, "\n");
    fprintf(stderr, "$ ");
}

int main(void) {
    char cmd[MAXLINE];
    char *av[MAXARGV];
    int status, ac;
    pid_t cpid;

    // SIGINTを受けたとき
    // シェルを終了せず,改行しプロンプトを表示する
    signal(SIGINT, handler);

    for (;;) {
        // プロンプトの表示,文字列受け取り
        fprintf(stderr, "$ ");
        fgets(cmd, MAXLINE, stdin);
        
        // トークナイズする
        if ((token = tokenize(cmd)) == NULL) {
            continue;
        }
        // パースする(二分木を作る)
        Node *node = expr();

        if ((cpid = fork()) == -1) {
            // fork error
            perror("fork");
            exit(1);
        } else if (cpid == 0) {
            // child process

            // 子プロセスグループがフォアグラウンドになるまで
            // 待つ
            while (tcgetpgrp(STDOUT_FILENO) != getpid()) {
                ;
            }
            // コマンド実行
            chexec(node);
            perror(cmd);
            exit(1);
        } else {
            // parent process
            // manよりtcsetpgrp()はを呼び出すと
            // バックグラウンドグループのすべてのメンバに
            // SIGTTOUが送られるので無視する
            signal(SIGTTOU, SIG_IGN);
            // child processをプロセスgipから外す
            if (setpgid(cpid, cpid) == -1) {
                perror("setpgid");
                exit(1);
            }
            // childprocessをフォアグラウンドにする
            if (tcsetpgrp(STDOUT_FILENO, cpid) == -1) {
                perror("tcsetpgrp");
                exit(1);
            }
            // 子プロセスが終了するまでブロック
            if (wait(&status) == (pid_t)-1) {
                perror("wait");
                exit(1);
            }
            // フォアグラウンドプロセスをシェルに戻す
            if (tcsetpgrp(STDOUT_FILENO, getpgrp())) {
                perror("tcsetpgrp");
                exit(1);
            }
        }
    }
    return 0;
}