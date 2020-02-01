#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

// 1回の操作のコマンド全体
#define MAXARGV 100


// tokenize.c
typedef enum {
    TK_RESERVED,    // 記号
    TK_WORD,        // コマンド(+オプション), ファイル
    TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind tkind;
    Token *next;
    char *str;
};

int splitspace (char*, char**, int);
Token *tokenize(char*);
int token_is_reserved(char*);


// parse.c
typedef enum {
    ND_CMD,             // コマンド(+オプション)
    ND_FILE,            // ファイル
    ND_REDSTDIN,        // <
    ND_REDSTDOUT,       // >
    ND_REDSTDERR,       // 2>
    ND_REDSTDOUT_PLUS,  // >>
    ND_REDSTDERR_PLUS,  // 2>>
    ND_PIPE,            // |
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind nkind;
    Node *left;
    Node *right;
    char *cmd[MAXARGV];
};

Token *token;   // 着目しているToken
Node *expr();