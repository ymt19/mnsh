#include "mnsh.h"

#define PATHNAME_SIZE 1024

int main(void) {
    char cmd[MAXLINE], cp_cmd[MAXLINE], path[PATHNAME_SIZE];
    int status, len;
    pid_t cpid, pid;
    Job *job;

    // ジョブリストを初期化
    job_tail = NULL;

    // 終了するシグナルのシグナルハンドラを指定
    signal(SIGINT, SIG_IGN);        /* ^C  */
    signal(SIGQUIT, SIG_IGN);       /* ^\  */
    signal(SIGTSTP, SIG_IGN);       /* ^Z  */
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    for (;;) {
        // プロンプトを表示する前に
        // バックグラウンドジョブで終了した子プロセスを
        // waitする
        for (;;) {
            if ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
                job = search_job_from_pgid(pid);
                fprintf(stderr, "[%d] Done    %s\n", job->job_num, job->cmd);
                free_job(job);
            } else if (pid == 0 || errno == ECHILD) {
                // waitpidするプロセスがない
                break;
            } else {
                perror("waitpid");
                exit(1);
            }
        }

        if (getcwd(path, sizeof(path)) == NULL) {
            perror("getcwd");
            exit(1);
        }

        // プロンプトの表示,文字列受け取り
        fprintf(stderr, "%s:$ ", path);
        fflush(stdin);
        fgets(cmd, MAXLINE, stdin);
        len = strlen(cmd);
        cmd[len-1] = '\0';

        // jobに渡すため,コピーする
        strncpy(cp_cmd, cmd, len);
        
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

                // 子プロセスグループがフォアグラウンドになるまで待つ
                while (tcgetpgrp(STDOUT_FILENO) != getpid()) {
                    ;
                }
                // コマンド実行
                chexec(node);
                perror(cmd);
                exit(1);
            } else {
                // parent process
                // jobリストに新しく追加する
                job = new_job(cp_cmd, cpid, Runnign);

                // child processをプロセスgipから外す
                if (setpgid(cpid, cpid) == -1) {
                    perror("setpgid");
                    exit(1);
                }

                // SIGTTOUを無視すれば,tcsetpgrpを呼び出しても
                // bgpgrpのすべてのメンバにはSIGTTOUは送られない
                // childprocessをフォアグラウンドプロセスにする
                signal(SIGTTOU, SIG_IGN);
                if (tcsetpgrp(STDOUT_FILENO, cpid) == -1) {
                    perror("tcsetpgrp");
                    exit(1);
                }

                if (node->nkind != ND_BG) {
                    // コマンドの実行にバックグラウンド実行の指定が無いとき
                    // forkした子プロセスが終了するまでブロック
                    // 停止した場合もwaitする
                    if (waitpid(cpid, &status, WUNTRACED) == -1) {
                        perror("waitpid");
                        exit(1);
                    }

                    if (WIFEXITED(status) || WIFSIGNALED(status)) {
                        // 正常終了またはシグナルによって終了した場合
                        // jobリストから削除
                        free_job(job);
                    } else {
                        // そうで無い場合,つまり一時中断した場合
                        // jobの状態をStoppedにする
                        set_jobstate(job, Stopped);
                    }

                    // フォアグラウンドプロセスをシェルに戻す
                    // バックグラウンドプロセスからの呼び出し
                    signal(SIGTTOU, SIG_IGN);
                    if (tcsetpgrp(STDOUT_FILENO, getpgrp()) == -1) {
                        perror("tcsetpgrp");
                        exit(1);
                    }
                } else {
                    // バックグラウンド実行の指定がある時
                    // 子プロセス側でtcsetpgrpするので
                    // 親プロセスがフォアグラウンドになるまで待つ
                    while (tcgetpgrp(STDOUT_FILENO) != getpid()) {
                        ;
                    }
                    fprintf(stderr, "[%d] PID:%d\n", job->job_num, job->pgid);
                }
            }
        }
    }
    return 0;
}