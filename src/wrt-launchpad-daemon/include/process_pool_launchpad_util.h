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
 * @file    process_pool_launchpad_util.h
 * @author  Tae-Jeong Lee (taejeong.lee@samsung.com)
 * @version 0.1
 * @brief   Api library to support launchpad operation.
 */

#ifndef __PROCESS_POOL_LAUNCHPAD_UTIL_H_
#define __PROCESS_POOL_LAUNCHPAD_UTIL_H_

#include <stdlib.h>
#include <launchpad_util.h>
#include <smack_labeling_support.h>
#include <execute_on_whole_thread_util.h>

#define DEBUG_NETWORK "system::debugging_network"

// Prototype
_static_ int    __process_pool_prepare_exec(const char *pkg_name, const char *app_path, app_info_from_db * menu_info, bundle * kb);
_static_ void   process_pool_launchpad_main_loop(app_pkt_t* pkt, char* out_app_path, int* out_argc, char ***out_argv);
_static_ void __set_prcoess_name(const char *apppath);

// Implementation
_static_ int __process_pool_prepare_exec(const char *pkg_name,
                            const char *app_path, app_info_from_db * menu_info,
                            bundle * kb)
{
    /* SET PRIVILEGES*/
    char pkg_id[PKG_ID_LENGTH];
    memset(pkg_id, '\0', PKG_ID_LENGTH);
    snprintf(pkg_id, PKG_ID_LENGTH, "%s", pkg_name);

    const char* is_debug = bundle_get_val(kb, "debug");
    if (is_debug != NULL && !strcmp(is_debug, "true")) {
        _D("Add system::debugging_network smack rule");
        __add_smack_rule(pkg_id, DEBUG_NETWORK, "rw");
        __add_smack_rule(DEBUG_NETWORK, pkg_id, "w");
    }

    if (set_app_smack_label(app_path) != 0)
    {
        _E("set_app_smack_label() failed");
    }

    if (__set_access(pkg_id, menu_info->pkg_type, app_path) < 0) {
        _D("fail to set privileges - check your package's credential\n");
        return -1;
    }

    /* SET INHERIT BIT FOR CAP_MAC_ADMIN TO WHOLE THREAD */
    EXECUTE_ON_WHOLE_THREAD(__set_inherit_bit_for_CAP_MAC_ADMIN, SIGUSR1);

    /* SET Process name */
    __set_prcoess_name(app_path);

    /* SET ENVIROMENT*/
    __set_env(menu_info, kb);

    return 0;
}


static bundle *_s_bundle = NULL;
static void __at_exit_to_release_bundle()
{
    if (_s_bundle) {
        bundle_free(_s_bundle);
        _s_bundle = NULL;
    }
}

_static_ void process_pool_launchpad_main_loop(app_pkt_t* pkt, char* out_app_path, int* out_argc, char ***out_argv)

{
    bundle *kb = NULL;
    app_info_from_db *menu_info = NULL;

    const char *pkg_name = NULL;
    const char *app_path = NULL;

    kb = bundle_decode(pkt->data, pkt->len);
    if (!kb) {
        _E("bundle decode error");
        exit(-1);
    }

    if (_s_bundle != NULL) {
        bundle_free(_s_bundle);
    }
    _s_bundle = kb;
    atexit(__at_exit_to_release_bundle);

    pkg_name = bundle_get_val(kb, AUL_K_PKG_NAME);
    SECURE_LOGD("pkg name : %s", pkg_name);

    menu_info = _get_app_info_from_bundle_by_pkgname(pkg_name, kb);
    if (menu_info == NULL) {
        _D("such pkg no found");
        exit(-1);
    }

    app_path = _get_app_path(menu_info);

    if (app_path == NULL) {
        _E("app_path is NULL");
        exit(-1);
    }

    if (app_path[0] != '/') {
        _E("app_path is not absolute path");
        exit(-1);
    }

    __modify_bundle(kb, /*cr.pid - unused parameter*/ 0, menu_info, pkt->cmd);
    pkg_name = _get_pkgname(menu_info);
    SECURE_LOGD("pkg name : %s", pkg_name);

    if (out_app_path != NULL && out_argc != NULL && out_argv != NULL)
    {
        int i;

        sprintf(out_app_path, "%s", app_path);

        *out_argv = __create_argc_argv(kb, out_argc);
        (*out_argv)[0] = out_app_path;

        for (i = 0; i < *out_argc; i++)
        {
            _D("input argument %d : %s##", i, (*out_argv)[i]);
        }
    }
    else
    {
        exit(-1);
    }

    __process_pool_prepare_exec(pkg_name, app_path, menu_info, kb);


    if (menu_info != NULL) {
        _free_app_info_from_db(menu_info);
    }
}

_static_ void __set_prcoess_name(const char *apppath){
    char cmdline[WRT_AUL_PR_NAME+1] = {0,};
    const char *pos = strrchr(apppath, '/');
    if( pos++ == NULL )
        pos = apppath;
    strncpy(cmdline, pos, WRT_AUL_PR_NAME);
    _D("apppath = %s", apppath);
    _D("cmdline = %s", cmdline);
    prctl(PR_SET_NAME, cmdline);
}


#endif // __PROCESS_POOL_LAUNCHPAD_UTIL_H_
