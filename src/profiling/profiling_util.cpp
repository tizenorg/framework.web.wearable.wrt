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
/*
 * @file
 * @author      Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version     1.0
 * @brief       This is implementation file for profiling function.
 */

#include "profiling_util.h"

#include <vector>
#include <cstdio>
#include <cstdlib>

#include <sys/time.h>
#include <signal.h>

#include <dpl/foreach.h>
#include <dpl/log/log.h>
#include <dpl/mutex.h>
#include <dpl/static_block.h>
#include <dpl/assert.h>

namespace {
const int PROFILING_OUTPUT_DESCRIPTOR = 3;

unsigned long long toULong(const struct timeval& value)
{
    unsigned long long ret = static_cast<unsigned long long>(value.tv_sec) *
        1000000ULL;
    ret += static_cast<unsigned long long>(value.tv_usec);
    return ret;
}

struct PacketResult
{
    unsigned long long time;
    const char* name;
    const char* prefix;
    const char* description;
    PacketResult() :
        time(0),
        name(0),
        prefix(0),
        description(0)
    {}
    PacketResult(unsigned long long t,
                 const char* n,
                 const char* p,
                 const char* d) :
        time(t),
        name(n),
        prefix(p),
        description(d)
    {}

    void Print(FILE *filePtr)
    {
        if (!prefix) {
            prefix = "";
        }
        if (!description) {
            description = "";
        }
        fprintf(filePtr, "%s#%s#%llu#[%s]\n", prefix, name, time, description);
    }
};

std::vector<PacketResult> results;

void dumpStatistic()
{
    FILE* fp = NULL;
    int newfd = dup(PROFILING_OUTPUT_DESCRIPTOR);
    if (-1 != newfd) {
        fp = fdopen(newfd, "w");
    }
    if (!fp) {
        if (-1 != newfd) {
            close(newfd);
        }
        fp = stdout; //fallback
    }
    fprintf(fp, "###PROFILING###START###\n");
    FOREACH(result, results)
    {
        result->Print(fp);
    }
    fprintf(fp, "###PROFILING###STOP###\n");
    fflush(fp);
    if (stdout != fp) {
        fclose(fp); // newfd is also closed
    }
}

void sigUsrHandler(int /*num*/)
{
    dumpStatistic();
}

DPL::Mutex* m_mutex = NULL;

void initialize()
{
    m_mutex = new DPL::Mutex;
    results.reserve(64 * 1024);
    signal(SIGUSR1, &sigUsrHandler);
    signal(SIGUSR2, &sigUsrHandler);
    LogDebug("Initialized profiling");
    AddProfilingMeasurment("Profiling_Started");
}

std::string GetFormattedTime()
{
    timeval tv;
    tm localNowTime;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &localNowTime);

    char format[64];
    snprintf(format,
             sizeof(format),
             "%02i:%02i:%02i.%03i",
             localNowTime.tm_hour,
             localNowTime.tm_min,
             localNowTime.tm_sec,
             static_cast<int>(tv.tv_usec / 1000));
    return format;
}
} // namespace anonymous

void AddStdoutProfilingMeasurment(const char* name, bool start)
{
    Assert(m_mutex != NULL);
    std::ostringstream output;
    output << "[" << GetFormattedTime() << "] [](): " << name << " ";
    output << (start ? "profiling##start" : "profiling##stop");
    fprintf(stdout, "%s\n", output.str().c_str());
}

#ifdef __cplusplus
extern "C"
#endif
void AddProfilingMeasurment(const char* name,
                            const char* prefix,
                            const char* description)
{
    DPL::Mutex::ScopedLock lock(m_mutex);
    struct timeval value;
    gettimeofday(&value, NULL);
    results.push_back(
        PacketResult(toULong(value), name, prefix, description));
}

STATIC_BLOCK
{
    initialize();
}

