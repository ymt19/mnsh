#include "mnsh.h"

// リダイレクトの処理
void do_redirect (Node *node) {
    int fd;
    char *fn;
    NodeKind nk = node->nkind;

    // ファイルオープン
    if (node->right->nkind == ND_FILE) {
        fn = node->right->cmd[0];
        if (nk == ND_REDSTDIN) {
            if ((fd = open(fn, O_RDONLY)) == -1) {
                perror("opne");
                exit(1);
            }
        } else if (nk == ND_REDSTDOUT || nk == ND_REDSTDERR) {
            if ((fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1) {
                perror("open");
                exit(1);
            }
        } else if (nk == ND_REDSTDOUT_PLUS || nk == ND_REDSTDERR_PLUS) {
            if ((fd = open(fn, O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1) {
                perror("open");
                exit(1);
            }
        }
        
    }

    // リダイレクト
    // それぞれのファイル記述子が
    // まだ書き換えられていない時にfdに変更
    if (nk == ND_REDSTDIN) {
    // 標準入力
        if ((dup2(fd, STDIN_FILENO)) == -1) {
            perror("dup2");
            exit(1);
        }
    } else if (nk == ND_REDSTDOUT || nk == ND_REDSTDOUT_PLUS) {
    // 標準出力
        if ((dup2(fd, STDOUT_FILENO)) == -1) {
            perror("dup2");
            exit(1);
        }
    } else if (nk == ND_REDSTDERR || nk == ND_REDSTDERR_PLUS) {
    // 標準エラー出力
        if ((dup2(fd, STDERR_FILENO)) == -1) {
            perror("dup2");
            exit(1);
        }
    }
}

// パイプライン
void do_pipeline (Node *node) {
    int fd[2], status;
    pid_t pid;
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    if ((pipe(fd)) == -1) {
        perror("pipe");
        exit(1);
    }

    if ((pid = fork()) == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
    // 子プロセス
    // 左部分木
        node = node->left;
        if ((dup2(fd[1], 1)) == -1) {
            perror("dup2");
            exit(1);
        }
        if (close(fd[0]) == -1) {
            perror("close");
            exit(1);
        }
        if (close(fd[1]) == -1) {
            perror("close");
            exit(1);
        }
        chexec(node);
    } else {
    // 親プロセス
    // 右部分木
        if (wait(&status) == (pid_t)-1) {
            perror("wait");
            exit(1);
        }

        node = node->right;
        if ((dup2(fd[0], 0)) == -1) {
            perror("dup2");
            exit(1);
        }
        if (close(fd[0]) == -1) {
            perror("close");
            exit(1);
        }
        if (close(fd[1]) == -1) {
            perror("close");
            exit(1);
        }
        chexec(node);
    }
}

// バックグラウンドジョブ
void do_background (Node *node) {
    // case1
    // 親プロセスでnodeからND_BGを含むかを判定し,waitするかを決める
    // 問題は,文法を付け足した時にND_BGを含むかを判定する
    // 必要が増える。現状は,ND_BGはnodeの根を見れば分かるので簡単

    pid_t ppid;
    ppid = getppid();
    // 親プロセス側で親プロセスグループをfgpgrpに
    // 変更すると,子プロセスのバックグラウンドジョブで実行された子プロセスが
    // 停止してしまったため,この場合のみ子プロセス側で親プロセスをfgpgrpnに戻した
    signal(SIGTTOU, SIG_IGN);
    if (tcsetpgrp(STDOUT_FILENO, ppid) == -1) {
        perror("tcsetpgrp");
        exit(1);
    }
    chexec(node->left);

    // // case2
    // // 孫プロセスを作り, 孫プロセスでコマンドを実行
    // // 子プロセスでexitする
    // // 問題は親プロセスが孫プロセスをwait出来ない
    // pid_t cpid;
    // if ((cpid = fork()) == -1) {
    //     perror("fork");
    //     exit(1);
    // } else if (cpid == 0) {
    //     // child process
    //     // 親プロセスグループと子プロセスグループが
    //     // 分けられるまで待つ
    //     while (tcgetpgrp(STDOUT_FILENO) == getpid()) {
    //         ;
    //     }
    //     // バックグラウンドでコマンドを実行する
    //     node = node->left;
    //     chexec(node);
    // } else {
    //     // parent process
    //     // cpidをプロセスグループから分ける
    //     if (setpgid(cpid, cpid) == -1) {
    //         perror("setpgid");
    //         exit(1);
    //     }
    //     // フォアグラウンドジョブ(親プロセス)は
    //     // 終了する
    //     exit(1);
    // }
}

void chexec (Node *node) {
    if (node->nkind == ND_CMD) {
        execvp(node->cmd[0], node->cmd);
        perror(node->cmd[0]);
        exit(1);
    } else if (node->nkind == ND_REDSTDIN) {
    // redirect()でそのノードと右部分木にあるファイル名を参照して
    // ファイル記述子を変更
        do_redirect(node);
        // 左部分木へ
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDOUT) {
        do_redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDERR) {
        do_redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDOUT_PLUS) {
        do_redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDERR_PLUS) {
        do_redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_PIPE) {
        do_pipeline(node);
    } else if (node->nkind == ND_BG) {
        do_background(node);
    }
}