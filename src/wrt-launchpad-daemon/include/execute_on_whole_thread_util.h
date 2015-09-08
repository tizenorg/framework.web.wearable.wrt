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
 * @file    execute_on_whole_thread_util.h
 * @author  Tae-Jeong Lee (taejeong.lee@samsung.com)
 * @version 1.0
 */

#ifndef __EXECUTE_ON_WHOLE_THREAD_UTIL_H__
#define __EXECUTE_ON_WHOLE_THREAD_UTIL_H__

#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/syscall.h>

// define
#define _EXEC_FILE_MAX_LEN 1024
#define _EXEC_MAX_RETRY_CNT 1000

// typedef
typedef void (*exec_func_t)();

// static variable
static exec_func_t  _s_exec_func    = NULL;
static sighandler_t _s_prev_handler = NULL;
static int _s_exec_waiting_task_cnt = 0;

// internal function
static void _exec_signal_handler(int signum)
{
    (void) signum;

    if (_s_exec_func)
    {
        _s_exec_func();
    }

    _s_exec_waiting_task_cnt--;
}

static int _set_exec_signal_handler(int signum)
{
    sighandler_t ret;

    ret = signal(signum, _exec_signal_handler);

    if (ret == SIG_ERR)
    {
        return -1;
    }
    else
    {
        _s_prev_handler = ret;
    }

    return 0;
}

static int _restore_signal_handler(int signum)
{
    if (_s_prev_handler)
    {
        if (signal(signum, _s_prev_handler) == SIG_ERR)
        {
            return -1;
        }

        _s_prev_handler = NULL;
    }
    else
    {
        if (signal(signum, SIG_DFL) == SIG_ERR)
        {
            return -1;
        }
    }

    return 0;
}

static int _send_signal_to_whole_thread(int signum)
{
    int ret;
    DIR *dir;
    struct dirent entry, *result;
    char proc_self_task_path[_EXEC_FILE_MAX_LEN + 1] = {0, };

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

            _s_exec_waiting_task_cnt++;
            if (syscall(__NR_tkill, atoi(entry.d_name), signum) != 0)
            {
                // syscall failed
                _s_exec_waiting_task_cnt--;
            }
        }

        closedir(dir);
    }
    else
    {
        return -1;
    }

    return 0;
}

static int _is_main_thread()
{
    int pid = getpid();
    int tid = syscall(__NR_gettid);

    return (pid == tid);
}

static int _waiting_for_done()
{
    int i;

    for (i=0; _s_exec_waiting_task_cnt && i < _EXEC_MAX_RETRY_CNT; i++)
    {
        usleep(100); // 0.1ms
    }

    if (i == _EXEC_MAX_RETRY_CNT)
    {
        // time over
        return -1;
    }

    return 0;
}

// external API
int EXECUTE_ON_WHOLE_THREAD(exec_func_t exec_func, int using_signum)
{
    int signum;

    assert(_s_exec_waiting_task_cnt == 0 && exec_func != NULL);
    assert(using_signum == SIGUSR1 || using_signum == SIGUSR2);

    // check main thread
    if (!_is_main_thread())
    {
        return -1;
    }

    signum       = using_signum;
    _s_exec_func = exec_func;

    // set signal handler
    if (_set_exec_signal_handler(signum) != 0)
    {
        goto onerr_EXECUTE_ON_WHOLE_THREAD;
    }

    // send signal
    if (_send_signal_to_whole_thread(signum) != 0)
    {
        goto onerr_EXECUTE_ON_WHOLE_THREAD;
    }

    // waiting
    if (_waiting_for_done() != 0)
    {
        goto onerr_EXECUTE_ON_WHOLE_THREAD;
    }

    // restore signal handler to previous
    _restore_signal_handler(signum);
    _s_exec_waiting_task_cnt = 0;
    _s_exec_func    = NULL;
    _s_prev_handler = NULL;

    return 0;


onerr_EXECUTE_ON_WHOLE_THREAD:
    _restore_signal_handler(signum);
    _s_exec_waiting_task_cnt = 0;
    _s_exec_func    = NULL;
    _s_prev_handler = NULL;

    return -1;
}

#endif // __EXECUTE_ON_WHOLE_THREAD_UTIL_H__
