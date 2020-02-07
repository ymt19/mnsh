#include "mnsh.h"

// 新しいジョブの情報を保存する
Job *new_job (char *cmd, pid_t pgid, JobState state) {
    Job *job = (Job*) malloc (sizeof(Job));
    job->pgid = pgid;
    job->state = state;

    // cmdはfgets()で受け取るので,最後尾に'\0'がある
    strncpy(job->cmd, cmd, MAXLINE);

    // ジョブ番号の設定
    if (job_tail == NULL) job->job_num = 1;
    else job->job_num = job_tail->job_num+1;

    // ジョブリストに繋ぐ
    job->next = job_tail;
    job_tail = job;
}

// ジョブ番号を用いて,同じジョブを指しているかの判定
// 真なら1,偽なら0を返す
int is_same_job (Job *a, Job *b) {
    if (a->job_num == b->job_num) {
        return 1;
    } else {
        return 0;
    }
}

// ジョブを解放する
void free_job (Job *free_job) {
    if (is_same_job(job_tail, free_job)) {
        // ジョブリストの最後尾をfreeする場合
        job_tail = free_job->next;
    } else {
        // それ以外
        Job *job;
        for (job = job_tail; job != NULL; job = job->next) {
            // ジョブリストからfree_jobの前のジョブを探す
            if (is_same_job(job->next, free_job)) {
                job->next = free_job->next;
                break;
            }
        }
    }
    free(free_job);
}

// ジョブの状態をセットする
void set_jobstate (Job *job, JobState state) {
    job->state = state;
}

// 格納されているジョブを表示する
// jobsコマンド
void print_joblist () {
    Job *job;
    char *str;
    for (job = job_tail; job != NULL; job = job->next) {
        fprintf(stdout, "[%d] ", job->job_num);
        fprintf(stdout, "%s", job->cmd);
        fprintf(stdout, "\n");
    }
}

// ジョブリストから指定されたジョブ番号のジョブを探す
Job *search_job_from_jobnum (int job_num) {
    Job *job;
    for (job = job_tail; job != NULL; job = job->next) {
        if (job_num == job->job_num) {
            break;
        }
    }
    return job;
}

// ジョブリストから指定されたプロセスグループIDのジョブを探す
Job *search_job_from_pgid (pid_t pgid) {
    Job *job;
    for (job = job_tail; job != NULL; job = job->next) {
        if (pgid == job->pgid) {
            break;
        }
    }
    return job;
}