#include "mnsh.h"

// 新しいジョブの情報を保存する
Job *new_job (char **cmd, pid_t pgid, JobState state) {
    Job *job = (Job*) malloc (sizeof(Job));
    job->cmd = cmd;
    job->pgid = pgid;
    job->state = state;
    // ジョブ番号の設定
    if (job_tail == NULL) job->job_num = 0;
    else job->job_num = job_tail->job_num+1;

    // ジョブリストに繋ぐ
    job->next = job_tail;
    job_tail = job;
}

// 格納されているジョブを表示する
// jobsコマンド
void print_joblist () {
    Job *job;
    char *str;
    for (job = job_tail; job != NULL; job = job->next) {
        fprintf(stdout, "[%d] ", job->job_num);
        for (str = job->cmd[0]; str != NULL; str++) {
            fprintf(stdout, "%s ", str);
        }
        fprintf(stdout, "\n");
    }
}

Job *search_job (int job_num) {
    Job *job;
    for (job = job_tail; job != NULL; job = job->next) {
        if (job_num == job->job_num) {
            break;
        }
    }
    return job;
}