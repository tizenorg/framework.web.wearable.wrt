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

#ifdef PRELOAD_ACTIVATE

#include <dlfcn.h>

#define PRELOAD_FILE        SHARE_PREFIX "/preload_list.txt"
#define PRELOAD_FILE_WRT    SHARE_PREFIX "/preload_list_wrt.txt"

#define EFL_PREINIT_FUNC    "elm_quicklaunch_init"
#define EFL_SHUTDOWN_FUNC   "elm_quicklaunch_shutdown"

static int preload_initialized = 0;
static int g_argc;
static char **g_argv;
static size_t max_cmdline_size = 0;

typedef struct handle_list_t {
    int (*dl_einit)();
    int (*dl_efini)();
    void *handle;
} handle_list_t;

static inline void __preload_init(int argc, char **argv)
{
    int i;

    g_argc = argc;
    g_argv = argv;
    for (i = 0; i < argc; i++) {
        max_cmdline_size += (strlen(argv[i]) + 1);
    }
    _D("max_cmdline_size = %d", max_cmdline_size);

    preload_initialized = 1;
}

/* TODO : how to set cmdline gracefully ?? */
static inline int __change_cmdline(char *cmdline)
{
    if (strlen(cmdline) > max_cmdline_size + 1) {
        _E("cmdline exceed max size : %d", max_cmdline_size);
        return -1;
    }

    memset(g_argv[0], '\0', max_cmdline_size);
    snprintf(g_argv[0], max_cmdline_size, "%s", cmdline);

    return 0;
}

static inline void __preload_exec(int argc, char **argv)
{
    void *handle = NULL;
    int (*dl_main)(int, char **);

    if (!preload_initialized) {
        return;
    }

    handle = dlopen(argv[0], RTLD_LAZY | RTLD_GLOBAL);
    if (handle == NULL) {
        _E("dlopen failed. bad preloaded app - check fpie pie");
        return;
    }

    dl_main = dlsym(handle, "main");
    if (dl_main != NULL) {
#ifndef NATIVE_LAUNCHPAD
        /* do nothing */
#else
        if (__change_cmdline(argv[0]) < 0) {
            _E("change cmdline fail");
            return;
        }
#endif
        dl_main(argc, argv);
    } else {
        _E("dlsym not founded. bad preloaded app - check fpie pie");
    }

    exit(0);
}

static int g_wrt_dlopen_size = 5;
static int g_wrt_dlopen_count = 0;
static void** g_wrt_dlopen_handle_list = NULL;

static inline int __preload_save_dlopen_handle(void *handle)
{
    if (!handle) {
        return 1;
    }
    if (g_wrt_dlopen_count == g_wrt_dlopen_size || !g_wrt_dlopen_handle_list) {
        void** tmp =
            realloc(g_wrt_dlopen_handle_list, 2 * g_wrt_dlopen_size * sizeof(void *));
        if (NULL == tmp) {
            _E("out of memory\n");
            dlclose(handle);
            return 1;
        }
        g_wrt_dlopen_size *= 2;
        g_wrt_dlopen_handle_list = tmp;
    }
    g_wrt_dlopen_handle_list[g_wrt_dlopen_count++] = handle;
    return 0;
}

static inline void __preload_fini_for_wrt()
{
    int i = 0;
    if (!g_wrt_dlopen_handle_list) {
        return;
    }
    for (i = 0; i < g_wrt_dlopen_count; ++i)
    {
        void *handle = g_wrt_dlopen_handle_list[i];
        if (handle) {
            if (0 != dlclose(handle)) {
                _E("dlclose failed\n");
            }
        }
    }
    free(g_wrt_dlopen_handle_list);
    g_wrt_dlopen_handle_list = NULL;
    g_wrt_dlopen_size = 5;
    g_wrt_dlopen_count = 0;
}

static inline void __preload_init_for_wrt()
{
    if (0 != atexit(__preload_fini_for_wrt)) {
        _E("Cannot register atexit callback. Libraries will not be unloaded");
    }
    void *handle = NULL;
    char soname[MAX_LOCAL_BUFSZ];
    FILE *preload_list;

    preload_list = fopen(PRELOAD_FILE_WRT, "rt");
    if (preload_list == NULL) {
        _E("no wrt preload\n");
        return;
    }

    while (fgets(soname, MAX_LOCAL_BUFSZ, preload_list) != NULL) {
        size_t len = strnlen(soname, MAX_LOCAL_BUFSZ);
        if (len > 0) {
            soname[len - 1] = '\0';
        }
        handle = dlopen(soname, RTLD_NOW | RTLD_GLOBAL);
        if (handle == NULL) {
            _E("dlopen(\"%s\") was failed!", soname);
            continue;
        }
        if (0 != __preload_save_dlopen_handle(handle)) {
            _E("Cannot save handle, no more preloads");
            break;
        }
        _D("preload %s# - handle : %x\n", soname, handle);
    }

    fclose(preload_list);
}

#else

static inline void __preload_init(int argc, char **argv);
static inline void __preload_exec(int argc, char **argv);
static inline void __preload_init_for_wrt();

#endif

