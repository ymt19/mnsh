#include "mnsh.h"

// 該当文字列str, 配列vec, 配列の最大格納数max
// parse.cで使うのでここに置くのは微妙
int splitspace (char *str, char **vec, int max) {
    int num, skip;
    if (max < 1 || vec == 0) {
        skip = 1;
    } else {
        skip = 0;
    }
    num = 0;
    while (1) {
        if (!skip && num >= max-1){
            vec[num] = 0;
            skip = 1; 
        }
        // 単語先頭を探す
        while (*str != '\0' && isspace(*str)) str++;
        if (*str == '\0') break;
        if (!skip) vec[num] = str;
        num++;
        // 単語末尾を探す
        while (*str != '\0' && !isspace(*str)) str++;
        if (*str = '\0') break;
        *str = '\0';
        str++;
    }
    if (!skip) vec[num] = NULL;
    num++;
    return num;
}

Token *new_token (TokenKind tkind, Token *cur, char *str, int len) {
    Token *tok = (Token*) malloc (sizeof(Token));
    tok->tkind = tkind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize (char *s) {
    // 記号とコマンドでトークンを作成
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*s) {
        if (isspace(*s)) {
            s++;
        } else if (strncmp(s, "2>>", 3) == 0) {
            cur = new_token(TK_RESERVED, cur, "2>>", 3);
            *s = '\0';
            s += 3;
        } else if (strncmp(s, ">>", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, ">>", 2);
            *s = '\0';
            s += 2;
        } else if (strncmp(s, "2>", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, "2>", 2);
            *s = '\0';
            s += 2;
        } else if (strncmp(s, ">", 1) == 0) {
            cur = new_token(TK_RESERVED, cur, ">", 1);
            *s = '\0';
            s++;
        } else if (strncmp(s, "<", 1) == 0) {
            cur = new_token(TK_RESERVED, cur, "<", 1);
            *s = '\0';
            s++;
        } else if (strncmp(s, "|", 1) == 0) {
            cur = new_token(TK_RESERVED, cur, "|", 1);
            *s = '\0';
            s++;
        } else if (strncmp(s, "&", 1) == 0) {
            cur = new_token(TK_RESERVED, cur, "&", 1);
            *s = '\0';
            s++;
        } else if (cur->tkind != TK_WORD) {
            cur = new_token(TK_WORD, cur, s, 0);
        } else {
            s++;
        }
    }

    // トークンが無いとき
    if (head.next == NULL) {
        return NULL;
    }
    new_token(TK_EOF, cur, s, 0);
    return head.next;
}

// 着目してるトークン(token)が指定した記号(TK_RESERVED)の時
// 1を返し、トークンを1つ読み進める
// それ以外の時、0を返す
// parse.cで使用
int token_is_reserved (char *s) {
    if (token->tkind != TK_RESERVED || strcmp(token->str, s) != 0) {
        return 0;
    }
    token = token->next;
    return 1;
}