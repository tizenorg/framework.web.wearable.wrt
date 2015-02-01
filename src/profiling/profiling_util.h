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
 * @brief       This is header file for profiling function.
 */

#ifndef WRT_ENGINE_SRC_PROFILING_PROFILING_UTIL_H
#define WRT_ENGINE_SRC_PROFILING_PROFILING_UTIL_H

#ifdef PROFILING_ENABLED

#ifndef __cplusplus
#include <stdbool.h>
#endif //__cplusplus

#ifdef __cplusplus
extern "C"
void AddProfilingMeasurment(const char* name,
                            const char* prefix = 0,
                            const char* description = 0);
#define ADD_PROFILING_POINT(name, ...) AddProfilingMeasurment(name, \
                                                              ##__VA_ARGS__)
#else //__cplusplus
void AddProfilingMeasurment(const char* name,
                            const char* prefix,
                            const char* description);
#define ADD_PROFILING_POINT(name, prefix, desc) AddProfilingMeasurment(name, \
                                                                       prefix, \
                                                                       desc)
#endif //__cplusplus

void AddStdoutProfilingMeasurment(const char* name, bool start);
// profiling script additional proceeds stdout output
#define LOG_PROFILE_START(x) AddStdoutProfilingMeasurment(x, true)
#define LOG_PROFILE_STOP(x) AddStdoutProfilingMeasurment(x, false)
#else //PROFILING_ENABLED

#define ADD_PROFILING_POINT(name, ...) (void)1
#define LOG_PROFILE_START(x) (void)1
#define LOG_PROFILE_STOP(x) (void)1

#endif //PROFILING_ENABLED

#endif /* WRT_ENGINE_SRC_PROFILING_PROFILING_UTIL_H */

