/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __SIMPLE_UTIL__
#define __SIMPLE_UTIL__

#include <unistd.h>
#include <ctype.h>
#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "WRT_LAUNCHPAD"


#define MAX_LOCAL_BUFSZ 128
#define MAX_PID_STR_BUFSZ 20

#undef _E
#define _E(fmt, arg ...) LOGE(fmt,##arg)
#undef _D
#define _D(fmt, arg ...) LOGD(fmt,##arg)

#ifndef SECURE_LOGD
#define SECURE_LOGD(fmt, arg...) LOGD(fmt,##arg)
#endif

#ifndef SECURE_LOGE
#define SECURE_LOGE(fmt, arg...) LOGE(fmt,##arg)
#endif

#define retvm_if(expr, val, fmt, arg ...) do { \
        if (expr) { \
            _E(fmt,##arg); \
            _E("(%s) -> %s() return", #expr, __FUNCTION__); \
            return (val); \
        } \
} while (0)

#define retv_if(expr, val) do { \
        if (expr) { \
            _E("(%s) -> %s() return", #expr, __FUNCTION__); \
            return (val); \
        } \
} while (0)

int __proc_iter_cmdline(int (*iterfunc)
                        (const char *dname, const char *cmdline, void *priv),
                        void *priv);
int __proc_iter_pgid(int pgid, int (*iterfunc)(int pid, void *priv),
                     void *priv);
char *__proc_get_cmdline_bypid(int pid);

static inline const char *FILENAME(const char *filename)
{
    const char *p;
    const char *r;

    if (!filename) {
        return NULL;
    }

    r = p = filename;
    while (*p) {
        if (*p == '/') {
            r = p + 1;
        }
        p++;
    }

    return r;
}

#endif