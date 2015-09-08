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
 * @file    launchpad_util.h
 * @author  Tae-Jeong Lee (taejeong.lee@samsung.com)
 * @version 0.1
 * @brief   Api library to support launchpad operation.
 */

#ifndef __LAUNCHPAD_UTIL_H_
#define __LAUNCHPAD_UTIL_H_

#include <aul.h>
#include <bundle.h>
#include <errno.h>
#include <privilege-control.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <unistd.h>

#include "config.h"
#include "gl.h"
#include "app_sock.h"
#include "menu_db_util.h"
#include "simple_util.h"
#include "access_control.h"

#define _static_ static inline
#define WRT_AUL_PR_NAME 16
#define PKG_ID_LENGTH   11
#define SDK_CODE_COVERAGE "CODE_COVERAGE"
#define SDK_DYNAMIC_ANALYSIS "DYNAMIC_ANALYSIS"
#define PATH_DA_SO "/home/developer/sdk_tools/da/da_probe.so"
#define PATH_APP_ROOT "/opt/usr/apps"
#define PATH_DATA "/data"

// Prototype
_static_ char** __create_argc_argv(bundle * kb, int *margc);
_static_ void   __set_sdk_env(app_info_from_db* menu_info, char* str);
_static_ int    __parser(const char *arg, char *out, int out_size);
_static_ void   __modify_bundle(bundle * kb, int caller_pid, app_info_from_db * menu_info, int cmd);
_static_ void   __set_env(app_info_from_db * menu_info, bundle * kb);
_static_ void   __set_inherit_bit_for_CAP_MAC_ADMIN();

// Implementation
_static_ char** __create_argc_argv(bundle * kb, int *margc)
{
    char **argv;
    int argc;

    argc = bundle_export_to_argv(kb, &argv);

    *margc = argc;
    return argv;
}

_static_ void __set_sdk_env(app_info_from_db* menu_info, char* str)
{
    char buf[MAX_LOCAL_BUFSZ];
    int ret;

    _D("key : %s / value : %s", AUL_K_SDK, str);
    /* http://gcc.gnu.org/onlinedocs/gcc/Cross_002dprofiling.html*/
    /* GCOV_PREFIX contains the prefix to add to the absolute paths in the
     *object file. */
    /*		Prefix can be absolute, or relative. The default is no prefix.
     * */
    /* GCOV_PREFIX_STRIP indicates the how many initial directory names */
    /*		to stripoff the hardwired absolute paths. Default value is 0. */
    if (strncmp(str, SDK_CODE_COVERAGE, strlen(str)) == 0) {
        snprintf(buf,
                 MAX_LOCAL_BUFSZ,
                 PATH_APP_ROOT "/%s"PATH_DATA,
                 _get_pkgname(menu_info));
        ret = setenv("GCOV_PREFIX", buf, 1);
        _D("GCOV_PREFIX : %d", ret);
        ret = setenv("GCOV_PREFIX_STRIP", "4096", 1);
        _D("GCOV_PREFIX_STRIP : %d", ret);
    } else if (strncmp(str, SDK_DYNAMIC_ANALYSIS, strlen(str)) == 0) {
        ret = setenv("LD_PRELOAD", PATH_DA_SO, 1);
        _D("LD_PRELOAD : %d", ret);
    }
}


/*
 * Parsing original app path to retrieve default bundle
 *
 * -1 : Invalid sequence
 * -2 : Buffer overflow
 *
 */
_static_ int __parser(const char *arg, char *out, int out_size)
{
    register int i;
    int state = 1;
    char *start_out = out;

    if (arg == NULL || out == NULL) {
        /* Handles null buffer*/
        return 0;
    }

    for (i = 0; out_size > 1; i++) {
        switch (state) {
        case 1:
            switch (arg[i]) {
            case ' ':
            case '\t':
                state = 5;
                break;
            case '\0':
                state = 7;
                break;
            case '\"':
                state = 2;
                break;
            case '\\':
                state = 4;
                break;
            default:
                *out = arg[i];
                out++;
                out_size--;
                break;
            }
            break;
        case 2:         /* escape start*/
            switch (arg[i]) {
            case '\0':
                state = 6;
                break;
            case '\"':
                state = 1;
                break;
            default:
                *out = arg[i];
                out++;
                out_size--;
                break;
            }
            break;
        case 4:         /* character escape*/
            if (arg[i] == '\0') {
                state = 6;
            } else {
                *out = arg[i];
                out++;
                out_size--;
                state = 1;
            }
            break;
        case 5:         /* token*/
            if (out != start_out) {
                *out = '\0';
                out_size--;
                return i;
            }
            i--;
            state = 1;
            break;
        case 6:
            return -1;                  /* error*/
        case 7:         /* terminate*/
            *out = '\0';
            out_size--;
            return 0;
        default:
            state = 6;
            break;              /* error*/
        }
    }

    if (out_size == 1) {
        *out = '\0';
    }
    /* Buffer overflow*/
    return -2;
}


_static_ void __modify_bundle(bundle * kb, int caller_pid,
                              app_info_from_db * menu_info, int cmd)
{
    // warning: unused parameter
    (void) caller_pid;

    bundle_del(kb, AUL_K_PKG_NAME);
    bundle_del(kb, AUL_K_EXEC);
    bundle_del(kb, AUL_K_PACKAGETYPE);
    bundle_del(kb, AUL_K_HWACC);

    /* Parse app_path to retrieve default bundle*/
    if (cmd == APP_START || cmd == APP_START_RES || cmd == APP_OPEN || cmd ==
        APP_RESUME)
    {
        char *ptr;
        char exe[MAX_PATH_LEN];
        int flag;

        ptr = _get_original_app_path(menu_info);

        flag = __parser(ptr, exe, sizeof(exe));
        if (flag > 0) {
            char key[256];
            char value[256];

            ptr += flag;
            _D("parsing app_path: EXEC - %s\n", exe);

            do {
                flag = __parser(ptr, key, sizeof(key));
                if (flag <= 0) {
                    break;
                }
                ptr += flag;

                flag = __parser(ptr, value, sizeof(value));
                if (flag < 0) {
                    break;
                }
                ptr += flag;

                /*bundle_del(kb, key);*/
                bundle_add(kb, key, value);
            } while (flag > 0);
        } else if (flag == 0) {
            _D("parsing app_path: No arguments\n");
        } else {
            _D("parsing app_path: Invalid argument\n");
        }
    }
}

_static_ void __set_env(app_info_from_db * menu_info, bundle * kb)
{
    const char *str;
    const char **str_array;
    int len;
    int i;

    char *pkgname = _get_pkgname(menu_info);
    if (pkgname == NULL) {
        _E("can't get pkgname");
        return;
    }
    setenv("PKG_NAME", pkgname, 1);

    USE_ENGINE("gl")

    str = bundle_get_val(kb, AUL_K_STARTTIME);
    if (str != NULL) {
        setenv("APP_START_TIME", str, 1);
    }

    if (bundle_get_type(kb, AUL_K_SDK) & BUNDLE_TYPE_ARRAY) {
        str_array = bundle_get_str_array(kb, AUL_K_SDK, &len);
        if (str_array != NULL) {
            for (i = 0; i < len; i++) {
                _D("index : [%d]", i);
                __set_sdk_env(menu_info, (char *)str_array[i]);
            }
        }
    } else {
        str = bundle_get_val(kb, AUL_K_SDK);
        if (str != NULL) {
            __set_sdk_env(menu_info, (char *)str);
        }
    }
    if (menu_info->hwacc != NULL) {
        setenv("HWACC", menu_info->hwacc, 1);
    }
}

_static_ void __set_inherit_bit_for_CAP_MAC_ADMIN()
{
    cap_t caps = NULL;
    cap_value_t target_caps[] = { CAP_MAC_ADMIN };

    caps = cap_init();

    if (caps == NULL) {
        goto err_set_inherit_bit_for_CAP_MAC_ADMIN;
    }

    if (cap_set_flag(caps, CAP_INHERITABLE,
                     sizeof(target_caps)/sizeof(cap_value_t),
                     target_caps, CAP_SET)) {
        _E("cap_set_flag() failed!! : %s", strerror(errno));
        goto err_set_inherit_bit_for_CAP_MAC_ADMIN;
    }

    if (cap_set_proc(caps)) {
        _E("cap_set_proc() failed!! : %s", strerror(errno));
        goto err_set_inherit_bit_for_CAP_MAC_ADMIN;
    }

    if (cap_free(caps)) {
        _E("cap_free() failed!!");
    }

    return;

err_set_inherit_bit_for_CAP_MAC_ADMIN:
    if (caps != NULL) {
        cap_free(caps);
    }

    return;
}

#endif // __LAUNCHPAD_UTIL_H_
