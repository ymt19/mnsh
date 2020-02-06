#include "mnsh.h"

// 実装済みビルトインコマンドなら値するBcmdKindを返し
// NOT_BCを返す
BcmdKind which_builtin (char *cmd) {
    if (strcmp(cmd, "exit") == 0) {
        return BC_EXIT;
    } else if (strcmp(cmd, "cd") == 0) {
        return BC_CD;
    } else if (strcmp(cmd, "bg") == 0) {
        return BC_BG;
    } else if (strcmp(cmd, "fg") == 0) {
        return BC_FG;
    } else if (strcmp(cmd, "stop") == 0) {
        return BC_STOP;
    } else {
        return NOT_BC;
    }
}


void do_exit();
void do_cd(Node*);

// bkindで指定された内部コマンドを実行する
void do_builtin (BcmdKind bkind, Node *node) {
    if (bkind == BC_EXIT) {
        do_exit();
    } else if (bkind == BC_CD) {
        do_cd(node);
    } else if (bkind == BC_BG) {
        // do_bg(node);
    } else if (bkind == BC_FG) {
        // do_fg(node);
    } else if (bkind == BC_STOP) {
        // do_stop(node);
    }
}

void do_exit() {
    fprintf(stderr, "GodBye!\n");
    exit(0);
}

void do_cd(Node *node) {
    if (node->ac > 2) {
        fprintf(stderr, "too many arguments\n");
    } else if (node->ac == 1) {
        // ホームディレクトリへ移動
        char *home = getenv("HOME");
        if (chdir(home) == -1) {
            perror("chdir");
            exit(1);
        }
    } else if (node->ac == 2) {
        // 引数のディレクトリへ移動
        if (chdir(node->cmd[1]) == -1) {
            perror("chdir");
            exit(1);
        }
    }
}