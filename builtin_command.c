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
    } else if (strcmp(cmd, "jobs") == 0) {
        return BC_JOBS;
    } else {
        return NOT_BC;
    }
}

void do_exit();
void do_cd(Node*);
void do_bg(Node*);
void do_fg(Node*);
void do_jobs();

// bkindで指定された内部コマンドを実行する
void do_builtin (BcmdKind bkind, Node *node) {
    if (bkind == BC_EXIT) {
        do_exit();
    } else if (bkind == BC_CD) {
        do_cd(node);
    } else if (bkind == BC_BG) {
        do_bg(node);
    } else if (bkind == BC_FG) {
        do_fg(node);
    } else if (bkind == BC_JOBS) {
        do_jobs();
    }
}

void do_exit() {
    fprintf(stderr, "GoodBye!\n");
    if (job_tail) {
        fprintf(stderr, "There are stopped jobs.\n");
    } else {
        exit(0);
    }
}

void do_cd(Node *node) {
    if (node->ac > 2) {
        fprintf(stderr, "cd: too many arguments\n");
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

void do_bg(Node *node) {
    int i;
    Job *job;
    // jobの優先順位の実装をしていないので
    // 引数の指定は必要
    if (node->ac == 1) {
        fprintf(stderr, "bg: usage: bg [number of job]\n");
        return;
    } else {
        for (i = 1; i < node->ac; i++) {
            int job_num = atoi(node->cmd[i]);
            // 引数で指定されたジョブ番号から,該当するジョブを探す
            if ((job = search_job_from_jobnum(job_num)) != NULL) {
                // 見つかって、かつ停止中だったらバックグラウンドで実行する
                if (job->state == Runnign) {
                    fprintf(stderr, "job %d alerady in background\n", job->job_num);
                } else {
                    job->state = Runnign;
                    fprintf(stderr, "[%d] Runnig", job->job_num);
                    killpg(job->pgid, SIGCONT);
                }
            } else {
                fprintf(stderr, "bg: %d: no such job\n", job_num);
            }
        }
    }
}

void do_fg(Node *node) {
    int i, status;
    Job *job;
    // jobの優先順位の実装をしていないので
    // 引数の指定は必要
    if (node->ac == 1) {
        fprintf(stderr, "fg: usage: fg [number of job]\n");
        return;
    } else {
        for (i = 1; i < node->ac; i++) {
            int job_num = atoi(node->cmd[i]);
            // 引数で指定されたジョブ番号から,該当するジョブを探す
            if ((job = search_job_from_jobnum(job_num)) != NULL) {
                job->state = Runnign;
                // 見つかったら、jobをフォアグラウンドで実行する
                // jobをfgpgrpにする
                signal(SIGTTOU, SIG_IGN);
                if (tcsetpgrp(STDOUT_FILENO, job->pgid) == -1) {
                    perror("tcsetpgrp");
                    exit(1);
                }
                // jobの実行を再開する
                killpg(job->pgid, SIGCONT);
                // プロセスグループリーダーが終了または停止しるまで待つ 
                if (waitpid(job->pgid, &status, WUNTRACED) == -1) {
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

                // fgpgrpを戻す
                signal(SIGTTOU, SIG_IGN);
                if (tcsetpgrp(STDOUT_FILENO, getpgrp()) == -1) {
                    perror("tcsetpgrp");
                    exit(1);
                }
            } else {
                fprintf(stderr, "fg: %d: no such job\n", job_num);
            }
        }
    }
}

void do_jobs() {
    print_joblist();
}