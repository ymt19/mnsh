#include "mnsh.h"

// リダイレクトの処理
void redirect (Node *node) {
    int fd;
    char *fn;
    NodeKind nk = node->nkind;

    // ファイルオープン
    if (node->right->nkind == ND_FILE) {
        fn = node->right->cmd[0];
        if (nk == ND_REDSTDIN) {
            if ((fd = open(fn, O_RDONLY)) == -1) {
                perror("opne");
                // exitしない方法を考える
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
        if (STDIN_FILENO == fileno(stdin)) {
            if ((dup2(fd, STDIN_FILENO)) == -1) {
                perror("dup2");
                exit(1);        // エラー処理
            }
        }
    } else if (nk == ND_REDSTDOUT || nk == ND_REDSTDOUT_PLUS) {
    // 標準出力
        if (STDOUT_FILENO == fileno(stdout)) {
            printf("1");
            dup2(fd, STDOUT_FILENO);
        }
    } else if (nk == ND_REDSTDERR || nk == ND_REDSTDERR_PLUS) {
    // 標準エラー出力
        if (STDERR_FILENO == fileno(stderr)) {
            dup2(fd, STDERR_FILENO);
        }
    }
}

// パイプライン
void pipeline (Node *node) {
    int fd[2];
    pid_t pid;
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);        // このエラー処理をどうするか
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

void chexec (Node *node) {
    if (node->nkind == ND_CMD) {
        execvp(node->cmd[0], node->cmd);
        perror(node->cmd[0]);
        exit(1);
    } else if (node->nkind == ND_REDSTDIN) {
    // redirect()でそのノードと右部分木にあるファイル名を参照して
    // ファイル記述子を変更
        redirect(node);
        // 左部分木へ
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDOUT) {
        redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDERR) {
        redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDOUT_PLUS) {
        redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_REDSTDERR_PLUS) {
        redirect(node);
        chexec(node->left);
    } else if (node->nkind == ND_PIPE) {
        pipeline(node);
    }
}