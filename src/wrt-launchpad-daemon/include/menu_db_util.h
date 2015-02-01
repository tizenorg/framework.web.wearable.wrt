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

#ifndef __MENU_DB_UTIL_H_
#define __MENU_DB_UTIL_H_

#include <pkgmgr-info.h>
#include <string.h>
#include "simple_util.h"

#define MAX_PATH_LEN    1024

#define AUL_APP_INFO_FLD_PKG_NAME               "package"
#define AUL_APP_INFO_FLD_APP_PATH               "exec"
#define AUL_APP_INFO_FLD_APP_TYPE               "x_slp_packagetype"
#define AUL_APP_INFO_FLD_WIDTH                  "x_slp_baselayoutwidth"
#define AUL_APP_INFO_FLD_HEIGHT                 "x_slp_baselayoutheight"
#define AUL_APP_INFO_FLD_VERTICAL               "x_slp_ishorizontalscale"
#define AUL_APP_INFO_FLD_MULTIPLE               "x_slp_multiple"
#define AUL_APP_INFO_FLD_TASK_MANAGE    "x_slp_taskmanage"
#define AUL_APP_INFO_FLD_MIMETYPE               "mimetype"
#define AUL_APP_INFO_FLD_SERVICE                "x_slp_service"

#define AUL_RETRIEVE_PKG_NAME                   "package = '?'"
#define AUL_RETRIEVE_APP_PATH                   "exec = '?'"
#define AUL_RETRIEVE_MIMETYPE                   "mimetype like '?'"
#define AUL_RETRIEVE_SERVICE                    "x_slp_service like '?'"

typedef struct {
    char *pkg_name;             /* package */
    char *app_path;             /* exec */
    char *original_app_path;            /* exec */
    char *pkg_type;             /* x_slp_packagetype */
    char *hwacc;                /* hwacceleration */
} app_info_from_db;

static inline char *_get_pkgname(app_info_from_db *menu_info)
{
    if (menu_info->pkg_name == NULL) {
        return NULL;
    }
    return menu_info->pkg_name;
}

static inline char *_get_app_path(app_info_from_db *menu_info)
{
    int i = 0;
    int path_len = -1;

    if (menu_info->app_path == NULL) {
        return NULL;
    }

    while (menu_info->app_path[i] != 0) {
        if (menu_info->app_path[i] == ' '
            || menu_info->app_path[i] == '\t')
        {
            path_len = i;
            break;
        }
        i++;
    }

    if (path_len == 0) {
        free(menu_info->app_path);
        menu_info->app_path = NULL;
    } else if (path_len > 0) {
        char *tmp_app_path = (char *)malloc(sizeof(char) * (path_len + 1));
        if (tmp_app_path == NULL) {
            return NULL;
        }
        snprintf(tmp_app_path, path_len + 1, "%s", menu_info->app_path);
        free(menu_info->app_path);
        menu_info->app_path = tmp_app_path;
    }

    return menu_info->app_path;
}

static inline char *_get_original_app_path(app_info_from_db *menu_info)
{
    if (menu_info->original_app_path == NULL) {
        return NULL;
    }
    return menu_info->original_app_path;
}

static inline void _free_app_info_from_db(app_info_from_db *menu_info)
{
    if (menu_info != NULL) {
        if (menu_info->pkg_name != NULL) {
            free(menu_info->pkg_name);
        }
        if (menu_info->app_path != NULL) {
            free(menu_info->app_path);
        }
        if (menu_info->original_app_path != NULL) {
            free(menu_info->original_app_path);
        }
        if (menu_info->hwacc != NULL) {
            free(menu_info->hwacc);
        }
        free(menu_info);
    }
}

static inline app_info_from_db *_get_app_info_from_db_by_pkgname(
    const char *pkgname)
{
    app_info_from_db *menu_info;
    pkgmgrinfo_appinfo_h  appinfo_handle;
    pkgmgrinfo_pkginfo_h  pkginfo_handle;
    int ret;
    char *str = NULL;

    menu_info = (app_info_from_db *)calloc(1, sizeof(app_info_from_db));
    if (menu_info == NULL) {
        return NULL;
    }

    ret = pkgmgrinfo_appinfo_get_appinfo(pkgname, &appinfo_handle);
    if (ret != PMINFO_R_OK ) {
        _E("error pkgmgrinfo_appinfo_get_appinfo(%s) : %d", pkgname, ret);
        _free_app_info_from_db(menu_info);
        return NULL;
    }

    ret = pkgmgrinfo_pkginfo_get_pkgid(appinfo_handle, &str);
    if (ret != PMINFO_R_OK ) {
        _E("pkgmgrinfo_pkginfo_get_pkgid failed");
    } else if (str) {
        menu_info->pkg_name = strdup(str);

        ret = pkgmgrinfo_pkginfo_get_pkginfo(str, &pkginfo_handle);
        if (ret != PMINFO_R_OK) {
            _E("error pkgmgrinfo_pkginfo_get_pkginfo(%s) : %d", str, ret);

            _free_app_info_from_db(menu_info);

            if (appinfo_handle != NULL) {
                ret = pkgmgrinfo_appinfo_destroy_appinfo(appinfo_handle);
                if (ret != PMINFO_R_OK ) {
                    _E("pkgmgrinfo_appinfo_destroy_appinfo failed");
                }
            }
            return NULL;
        }

        ret = pkgmgrinfo_pkginfo_get_type(pkginfo_handle, &str);
        if (ret != PMINFO_R_OK ) {
            _E("pkgmgrinfo_pkginfo_get_type failed");
        } else if (str) {
            menu_info->pkg_type = strdup(str);
        }

        if (pkginfo_handle != NULL) {
            ret = pkgmgrinfo_pkginfo_destroy_pkginfo(pkginfo_handle);
            if (ret != PMINFO_R_OK ) {
                _E("pkgmgrinfo_appinfo_destroy_pkginfo failed");
            }
        }

        str = NULL;
    }

    ret = pkgmgrinfo_appinfo_get_exec(appinfo_handle, &str);
    if (ret != PMINFO_R_OK ) {
        _E("pkgmgrinfo_appinfo_get_exec failed");
    } else if (str) {
        menu_info->app_path = strdup(str);
        str = NULL;
    }

    if (menu_info->app_path != NULL) {
        menu_info->original_app_path = strdup(menu_info->app_path);
    }

    if (appinfo_handle != NULL) {
        ret = pkgmgrinfo_appinfo_destroy_appinfo(appinfo_handle);
        if (ret != PMINFO_R_OK ) {
            _E("pkgmgrinfo_appinfo_destroy_appinfo failed");
        }
    }

    if (!_get_app_path(menu_info)) {
        _free_app_info_from_db(menu_info);
        return NULL;
    }

    return menu_info;
}

static inline int  __appinfo_func(const pkgmgrinfo_appinfo_h appinfo,
                                          void *user_data)
{
    app_info_from_db *menu_info = (app_info_from_db *)user_data;
    char *package;

    int ret = pkgmgrinfo_appinfo_get_pkgid(appinfo, &package);
    if (ret != PMINFO_R_OK ) {
        _E("pkgmgrinfo_appinfo_get_pkgid failed");
        return -1;
    }
    menu_info->pkg_name = strdup(package);
    return 0;
}

static inline app_info_from_db *_get_app_info_from_db_by_apppath(
    const char *apppath)
{
    app_info_from_db *menu_info = NULL;
    pkgmgrinfo_appinfo_filter_h filter;
    int ret;
    int count;

    if (apppath == NULL) {
        return NULL;
    }

    menu_info = (app_info_from_db *)calloc(1, sizeof(app_info_from_db));
    if (menu_info == NULL) {
        return NULL;
    }

    ret = pkgmgrinfo_appinfo_filter_create(&filter);
    if (ret != PMINFO_R_OK ) {
        _free_app_info_from_db(menu_info);
        return NULL;
    }

    ret = pkgmgrinfo_appinfo_filter_add_string(filter, PMINFO_APPINFO_PROP_APP_EXEC, apppath);
    if (ret != PMINFO_R_OK ) {
        ret = pkgmgrinfo_appinfo_filter_destroy(filter);
        if (ret != PMINFO_R_OK ) {
            _E("pkgmgrinfo_appinfo_filter_destroy failed");
        }
        _free_app_info_from_db(menu_info);
        return NULL;
    }

    ret = pkgmgrinfo_appinfo_filter_count(filter, &count);
    if (ret != PMINFO_R_OK ) {
        ret = pkgmgrinfo_appinfo_filter_destroy(filter);
        if (ret != PMINFO_R_OK ) {
            _E("pkgmgrinfo_appinfo_filter_destroy failed");
        }
        _free_app_info_from_db(menu_info);
        return NULL;
    }
    if (count < 1) {
        ret = pkgmgrinfo_appinfo_filter_destroy(filter);
        if (ret != PMINFO_R_OK ) {
            _E("pkgmgrinfo_appinfo_filter_destroy failed");
        }
        _free_app_info_from_db(menu_info);
        return NULL;
    }

    ret = pkgmgrinfo_appinfo_filter_foreach_appinfo(filter, __appinfo_func, (void *)menu_info);
    if (ret != PMINFO_R_OK ) {
        _E("pkgmgrinfo_appinfo_filter_foreach_appinfo failed");
    }

    ret = pkgmgrinfo_appinfo_filter_destroy(filter);
    if (ret != PMINFO_R_OK ) {
        _E("pkgmgrinfo_appinfo_filter_destroy failed");
    }

    menu_info->app_path = strdup(apppath);
    menu_info->original_app_path = strdup(apppath);

    return menu_info;
}


static inline app_info_from_db *_get_app_info_from_bundle_by_pkgname(
    const char *pkgname, bundle *kb)
{
    app_info_from_db *menu_info;

    menu_info = (app_info_from_db*)calloc(1, sizeof(app_info_from_db));
    if (menu_info == NULL) {
        return NULL;
    }

    menu_info->pkg_name = strdup(pkgname);
    menu_info->app_path = strdup(bundle_get_val(kb, AUL_K_EXEC));
    if (menu_info->app_path != NULL) {
        menu_info->original_app_path = strdup(menu_info->app_path);
    }
    menu_info->pkg_type = strdup(bundle_get_val(kb, AUL_K_PACKAGETYPE));
    menu_info->hwacc = strdup(bundle_get_val(kb, AUL_K_HWACC));

    if (!_get_app_path(menu_info)) {
        _free_app_info_from_db(menu_info);
        return NULL;
    }

    return menu_info;
}
#endif //__MENU_DB_UTIL_H_