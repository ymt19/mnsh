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
    TK_CMD,         // コマンド
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