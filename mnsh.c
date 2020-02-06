#include "mnsh.h"

#define MAXLINE 4096

// プロンプトの再表示
void prompt_reset_handler (int sig) {
    fprintf(stderr, "\n");
    fprintf(stderr, "$ ");
    fflush(stdin);
}

int main(void) {
    char cmd[MAXLINE];
    int status;
    pid_t cpid, pid;

    // 終了するシグナルのシグナルハンドラを指定
    signal(SIGINT, prompt_reset_handler);       /* ^C  */
    signal(SIGQUIT, prompt_reset_handler);      /* ^\  */
    signal(SIGTSTP, prompt_reset_handler);      /* ^Z  */
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    for (;;) {
        // プロンプトを表示する前に
        // バックグラウンドジョブで終了した子プロセスを
        // waitする
        for (;;) {
            if ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                fprintf(stderr, "PID:[%d] terminated\n", pid);
            } else if (pid == 0 || errno == ECHILD) {
                // waitpidするプロセスがない
                break;
            } else {
                perror("waitpid");
                exit(1);
            }
        }

        // プロンプトの表示,文字列受け取り
        fprintf(stderr, "$ ");
        fflush(stdin);
        fgets(cmd, MAXLINE, stdin);
        
        // 入力文字列をトークナイズする
        if ((token = tokenize(cmd)) == NULL) {
            continue;
        }
        // パースする(二分木を作る)
        Node *node = expr();

        // ビルトインコマンドならforkしない
        // そうでないなら,forkして子プロセスで実行
        BcmdKind bkind;
        if (is_nkind(node, ND_CMD) && (bkind = which_builtin(node->cmd[0])) != NOT_BC) {
            do_builtin(bkind, node);
        } else {
            // forkして
            // 子プロセスをコマンドの実行に使用する
            if ((cpid = fork()) == -1) {
                // fork error
                perror("fork");
                exit(1);
            } else if (cpid == 0) {
                // child process
                // シグナルハンドラの設定をデフォルトに戻す
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);

                // 子プロセスグループがフォアグラウンドプロセスになるまで
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
                // child processをプロセスgipから外す
                if (setpgid(cpid, cpid) == -1) {
                    perror("setpgid");
                    exit(1);
                }

                // SIGTTOUを無視すれば,tcsetpgrpを呼び出しても
                // bgpgrpのすべてのメンバにはSIGTTOUは送られない
                // childprocessをフォアグラウンドプロセスにする
                if (tcsetpgrp(STDOUT_FILENO, cpid) == -1) {
                    perror("tcsetpgrp");
                    exit(1);
                }

                // コマンドの実行にバックグラウンド実行の指定が無いとき
                // forkした子プロセスが終了するまでブロック
                if (node->nkind != ND_BG) {
                    if (waitpid(cpid, &status, WUNTRACED) == -1) {
                        perror("waitpid");
                        exit(1);
                    }

                    // フォアグラウンドプロセスをシェルに戻す
                    // バックグラウンドプロセスからの呼び出し
                    if (tcsetpgrp(STDOUT_FILENO, getpgrp()) == -1) {
                        perror("tcsetpgrp");
                        exit(1);
                    }
                }
            }
        }
    }
    return 0;
}