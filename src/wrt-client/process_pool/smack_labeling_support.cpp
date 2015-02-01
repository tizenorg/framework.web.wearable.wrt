/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file    smack_labeling_support.cpp
 * @author  Tae-Jeong Lee (taejeong.lee@samsung.com)
 * @version 0.1
 * @brief   API to support smack labeling for whole threads in a process.
 */

#include "smack_labeling_support.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/smack.h>
#include <dirent.h>
#include <assert.h>
#include <dpl/log/log.h>

#define SMACK_LABEL_LEN 255
#define FILE_MAX_LEN    1024
#define MAX_RETRY_CNT   1000
#define UID_ROOT        0

static char s_smack_label[SMACK_LABEL_LEN + 1] = {0,};
static int  s_waiting_task_cnt = 0;

static int smack_set_label_for_tid(const char *label)
{
    int len, fd, ret;
    char curren_path[FILE_MAX_LEN + 1] = {0,};

    len = strnlen(label, SMACK_LABEL_LEN + 1);

    if (len > SMACK_LABEL_LEN)
    {
        return -1;
    }

    snprintf(curren_path, sizeof(curren_path), "/proc/%d/attr/current", (int)syscall(__NR_gettid));

    fd = open(curren_path, O_WRONLY);

    if (fd < 0)
    {
        return -1;
    }

    ret = write(fd, label, len);
    close(fd);

    return (ret < 0) ? -1 : 0;
}

static void SIGUSR1_handler(int /*signo*/)
{
    if (smack_set_label_for_tid(s_smack_label) != 0)
    {
        LogError("## [tid:" << syscall(__NR_gettid) << "] smack_set_label_for_tid() failed! ##");
    }
    s_waiting_task_cnt--;
}

static int set_SIGUSR1_handler()
{
    if (signal(SIGUSR1, SIGUSR1_handler) == SIG_ERR)
    {
        LogError("## signal(SIGUSR1, SIGUSR1_handler) failed! ##");
        return -1;
    }

    return 0;
}

static int set_SIGUSR1_to_default()
{
    if (signal(SIGUSR1, SIG_DFL) == SIG_ERR)
    {
        LogError("## signal(SIGUSR1, SIG_ERR) failed! ##");
        return -1;
    }

    return 0;
}

static int send_SIGUSR1_to_threads()
{
    int ret;
    DIR *dir;
    struct dirent entry, *result;
    char proc_self_task_path[FILE_MAX_LEN + 1] = {0, };

    sprintf(proc_self_task_path, "/proc/self/task");

    dir = opendir(proc_self_task_path);

    if (dir)
    {
        for (ret = readdir_r(dir, &entry, &result);
             result != NULL && ret == 0;
             ret = readdir_r(dir, &entry, &result))
        {
            if (strncmp(entry.d_name, ".", 2) == 0 ||
                strncmp(entry.d_name, "..", 3) == 0)
            {
                continue;
            }

            s_waiting_task_cnt++;
            if (syscall(__NR_tkill, atoi(entry.d_name), SIGUSR1) != 0)
            {
                LogError("## tkill(" << atoi(entry.d_name) << "SIGUSR1) failed! ##");
                s_waiting_task_cnt--;
            }
        }

        closedir(dir);
    }
    else
    {
        LogError("## opendir(\"/proc/self/task\") failed! ##");
        return -1;
    }

    return 0;
}

int set_app_smack_label(const char* app_path)
{
    assert(s_waiting_task_cnt == 0);

    if (UID_ROOT != getuid() || app_path == NULL)
    {
        LogError("## parameter error! ##");
        return -1;
    }

    // set SIGUSR1 signal handler
    if (set_SIGUSR1_handler() != 0)
    {
        return -1;
    }

    // get smack label from app_path
    char *smack_label = NULL;

    if (smack_lgetlabel(app_path, &smack_label, SMACK_LABEL_EXEC) != 0)
    {
        LogError("## smack_lgetlabel() failed! ##");
        goto err_set_app_smack_label;
    }

    if (smack_label)
    {
        strncpy(s_smack_label, smack_label, sizeof(s_smack_label));
        s_smack_label[SMACK_LABEL_LEN] = '\0';

        free(smack_label);
        smack_label = NULL;
    }
    else
    {
        LogError("## smack_label is NULL! ##");
        strcpy(s_smack_label, "");
    }

    if (send_SIGUSR1_to_threads() != 0)
    {
        LogError("## send_SIGUSR1_to_threads() timeout! ##");
        goto err_set_app_smack_label;
    }

    // wait for labeling on each tasks
    int i;

    for (i=0; s_waiting_task_cnt && i < MAX_RETRY_CNT; i++)
    {
        usleep(100); // 0.1ms
    }

    if (i == MAX_RETRY_CNT)
    {
        LogError("## set_app_smack_label() timeout! ##");
    }

    // set SIGUSR1 signal defualt handler
    set_SIGUSR1_to_default();

    return 0;

err_set_app_smack_label:
    s_waiting_task_cnt = 0;
    set_SIGUSR1_to_default();

    return -1;
}


