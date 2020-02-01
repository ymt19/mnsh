#include "mnsh.h"

Node *new_node (NodeKind nkind, Node *left, Node *right, char *s) {
    Node *node = (Node*) malloc (sizeof(Node));
    node->nkind = nkind;
    node->left = left;
    node->right = right;

    int ac;
    if (nkind == ND_CMD) {
        // execvpのためにコマンドをベクタ配列にする
        if ((ac == splitspace(s, node->cmd, MAXARGV)) > MAXARGV) {
            fprintf(stderr, "%s: too many argments.\n", node->cmd[0]);      // ここのエラーを処理しきれてない
            return NULL;
        }
    } else if (nkind == ND_FILE) {
        // 最後尾のNULLとファイル名のみ格納される
        if ((ac == splitspace(s, node->cmd, 2)) > 2) {
            fprintf(stderr, "%s: expect filename.\n", s);
            return NULL;
        }
    }
    return node;
}

// 再帰下降構文解析
Node *file();
Node *cmd();

// expr = cmd ('<' file | '>' file | '2>' file | '>>' file | '2<<' file)
Node *expr () {
    Node *node = cmd();
    for (;;) {
        if (token_is_reserved("<")) {
            node = new_node(ND_REDSTDIN, node, file(), NULL);
        } else if (token_is_reserved(">")) {
            node = new_node(ND_REDSTDOUT, node, file(), NULL);
        } else if (token_is_reserved("2>")) {
            node = new_node(ND_REDSTDERR, node, file(), NULL);
        } else if (token_is_reserved(">>")) {
            node = new_node(ND_REDSTDOUT_PLUS, node, file(), NULL);
        } else if (token_is_reserved("2>>")) {
            node = new_node(ND_REDSTDERR_PLUS, node, file(), NULL);
        } else {
            return node;
        }
    }
}

Node *file() {
    Node *node = new_node(ND_FILE, NULL, NULL, token->str);
    token = token->next;
    return node;
}

Node *cmd() {
    Node *node = new_node(ND_CMD, NULL, NULL, token->str);
    token = token->next;
    return node;
}