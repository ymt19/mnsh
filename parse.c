#include "mnsh.h"

Node *new_node (NodeKind nkind, Node *left, Node *right, char *s) {
    Node *node = (Node*) malloc (sizeof(Node));
    node->nkind = nkind;
    node->left = left;
    node->right = right;

    int ac;
    if (nkind == ND_CMD) {
        // execvpのためにコマンドをベクタ配列にする
        if ((ac = splitspace(s, node->cmd, MAXARGV)) > MAXARGV) {
            fprintf(stderr, "%s: too many argments.\n", node->cmd[0]);      // ここのエラーを処理しきれてない
            return NULL;
        }
        ac--;
        node->ac = ac;
    } else if (nkind == ND_FILE) {
        // 最後尾のNULLとファイル名のみ格納される
        if ((ac == splitspace(s, node->cmd, 2)) > 2) {
            fprintf(stderr, "%s: expect filename.\n", s);
            return NULL;
        }
    }
    return node;
}

// nodeの種類が第二引数と一致したら
// 1を返す, それ以外なら0を返す
int is_nkind (Node *node, NodeKind nkind) {
    if (node->nkind == nkind) {
        return 1;
    } else {
        return 0;
    }
}

// 再帰下降構文解析
Node *pipeline();
Node *redirection();
Node *file();
Node *cmd();

// expr = pipeline '&'?  
Node *expr() {
    Node *node = pipeline();
    if (token_is_reserved("&")) {
        node = new_node(ND_BG, node, NULL, NULL);
    }
    return node;
}

// pipeline = redirection ('|' redirection)*
Node *pipeline() {
    Node *node = redirection();
    for (;;) {
        if (token_is_reserved("|")) {
            node = new_node(ND_PIPE, node, redirection(), NULL);
        } else {
            return node;
        }
    }
}

// redirection = cmd ('<' file | '>' file | '2>' file | '>>' file | '2<<' file)*
Node *redirection () {
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

// file = word
Node *file() {
    Node *node = new_node(ND_FILE, NULL, NULL, token->str);
    token = token->next;
    return node;
}

// cmd = word
Node *cmd() {
    Node *node = new_node(ND_CMD, NULL, NULL, token->str);

    token = token->next;
    return node;
}