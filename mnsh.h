#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>

// コマンドとその引数を合わせた上限
#define MAXARGV 100
// コマンドラインの長さの制限
#define MAXLINE 4096


/* tokenize.c */
// トークンの種類
typedef enum {
    TK_RESERVED,    // 記号
    TK_WORD,        // コマンド(+オプション), ファイル
    TK_EOF,
} TokenKind;

// トークン
typedef struct Token Token;
struct Token {
    TokenKind tkind;
    Token *next;
    char *str;
};

int splitspace (char*, char**, int);
Token *tokenize(char*);
int token_is_reserved(char*);


/* parse.c */
// 二分木のノードの種類
typedef enum {
    ND_CMD,             // コマンド(+オプション)
    ND_FILE,            // ファイル
    ND_REDSTDIN,        // <
    ND_REDSTDOUT,       // >
    ND_REDSTDERR,       // 2>
    ND_REDSTDOUT_PLUS,  // >>
    ND_REDSTDERR_PLUS,  // 2>>
    ND_PIPE,            // |
    ND_BG,              // &
} NodeKind;

// 二分木のノード
typedef struct Node Node;
struct Node {
    NodeKind nkind;
    Node *left;
    Node *right;
    char *cmd[MAXARGV];
    int ac;
};

Token *token;   // 着目しているToken
int is_nkind(Node*, NodeKind);
Node *expr();


/* chexec.c */
void chexec(Node*);

/* builtin_cmmand.c */
// ビルトインコマンドの種類
typedef enum {
    BC_EXIT,
    BC_CD,
    BC_BG,
    BC_FG,
    BC_JOBS,
    NOT_BC,         // ビルトインコマンドではない
} BcmdKind;

// 与えられた引数が
// どのビルトインコマンドかを判定する
BcmdKind which_builtin (char*);

// ビルトインコマンドを実行する
void do_builtin (BcmdKind, Node*);


/* job_control.c */
// ジョブの状態
typedef enum {
    Runnign,
    Stopped,
} JobState;

// ジョブの単位
typedef struct Job Job;
struct Job {
    struct Job *next;   // 次のジョブ
    char cmd[MAXLINE];  // メッセージに使用
    pid_t pgid;         // プロセスグループID
    JobState state;     // ジョブの状態
    int job_num;        // ジョブ番号
};

// ジョブリストの末尾
Job *job_tail;

Job *new_job(char*, pid_t, JobState);
int is_same_job (Job*, Job*);
void free_job (Job*);
void set_jobstate (Job*, JobState);
void print_joblist(void);
Job *search_job_from_jobnum (int);
Job *search_job_from_pgid (pid_t);