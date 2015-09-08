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
 * @file    client_security_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "client_security_support.h"

#include <string>

#include <pkgmgr-info.h>
#include <privilege-control.h>

#include <dpl/exception.h>
#include <dpl/log/secure_log.h>

namespace ClientModule {
namespace {
class Exception
{
public:
    DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
    DECLARE_EXCEPTION_TYPE(Base, GetAppInfoFailed)
    DECLARE_EXCEPTION_TYPE(Base, GetPkgInfoFailed)
    DECLARE_EXCEPTION_TYPE(Base, GetAppInfoStrFailed)
    DECLARE_EXCEPTION_TYPE(Base, SetPrivilegeFailed)
};

// Function declare
void destroyAppInfoHandle(pkgmgrinfo_appinfo_h handle);
void getAppInfo(const std::string& tizenAppId, pkgmgrinfo_appinfo_h* handle);
void getPkgInfo(const char * tizenPkgId, pkgmgrinfo_pkginfo_h *handle);
char* getExePath(pkgmgrinfo_appinfo_h handle);
char* getPackageId(pkgmgrinfo_appinfo_h handle);
char* getPackageType(const char* pkgId);

void destroyAppInfoHandle(pkgmgrinfo_appinfo_h handle)
{
    if (handle != NULL) {
        int ret = pkgmgrinfo_appinfo_destroy_appinfo(handle);
        if (ret != PMINFO_R_OK) {
            _E("pkgmgrinfo_appinfo_destroy_appinfo failed");
        }
    }
}

void destroyPkgInfoHandle(pkgmgrinfo_pkginfo_h handle)
{
    if (handle != NULL) {
        int ret = pkgmgrinfo_pkginfo_destroy_pkginfo(handle);
        if (ret != PMINFO_R_OK) {
            _E("pkgmgrinfo_pkginfo_destroy_pkginfo failed");
        }
    }
}

void getAppInfo(const std::string& tizenAppId, pkgmgrinfo_appinfo_h* handle)
{
    int ret = pkgmgrinfo_appinfo_get_appinfo(tizenAppId.c_str(), handle);
    if (ret != PMINFO_R_OK) {
        _E("error pkgmgrinfo_appinfo_get_appinfo(%s) : %d", tizenAppId.c_str(), ret);
        Throw(Exception::GetAppInfoFailed);
    }
}

void getPkgInfo(const char * tizenPkgId, pkgmgrinfo_pkginfo_h *handle){
    int ret = pkgmgrinfo_pkginfo_get_pkginfo(tizenPkgId, handle);
    if( ret != PMINFO_R_OK ){
        _E("error pkgmgrinfo_pkginfo_get_pkginfo(%s) : %d", tizenPkgId, ret);
        Throw(Exception::GetPkgInfoFailed);
    }
}

char* getExePath(pkgmgrinfo_appinfo_h handle)
{
    char* str = NULL;
    int ret = pkgmgrinfo_appinfo_get_exec(handle, &str);
    if (ret != PMINFO_R_OK) {
        _E("error pkgmgrinfo_appinfo_get_exec(%s) : %d", PMINFO_APPINFO_PROP_APP_EXEC, ret);
        Throw(Exception::GetAppInfoStrFailed);
    }
    return str;
}

char* getPackageId(pkgmgrinfo_appinfo_h handle)
{
    char* str = NULL;
    int ret = pkgmgrinfo_appinfo_get_pkgid(handle, &str);
    if (ret != PMINFO_R_OK) {
        _E("error pkgmgrinfo_appinfo_get_pkgid(%s) : %d", PMINFO_PKGINFO_PROP_PACKAGE_ID, ret);
        Throw(Exception::GetAppInfoStrFailed);
    }
    return str;
}

char* getPackageType(pkgmgrinfo_pkginfo_h handle)
{
    char* str = NULL;
    int ret = pkgmgrinfo_pkginfo_get_type(handle, &str);
    if (ret != PMINFO_R_OK) {
        _E("error pkgmgrinfo_pkginfo_get_type(%s) : %d", PMINFO_PKGINFO_PROP_PACKAGE_TYPE, ret);
        Throw(Exception::GetAppInfoStrFailed);
    }
    return str;
}

} // namespace anonymous

bool SecuritySupport::setAppPrivilege(const std::string& tizenAppId)
{
    pkgmgrinfo_appinfo_h handle = NULL;
    pkgmgrinfo_pkginfo_h pkg_handle = NULL;
    Try
    {
        getAppInfo(tizenAppId, &handle);
        char* path = getExePath(handle);
        char* pkgId = getPackageId(handle);
        getPkgInfo(pkgId,&pkg_handle);
        char* type = getPackageType(pkg_handle);

        _D("Package ID   : %s", pkgId);
        _D("Package TYPE : %s", type);
        _D("Package PATH : %s", path);

        int ret = perm_app_set_privilege(pkgId, type, path);
        if (ret != PC_OPERATION_SUCCESS) {
            _E("error perm_app_set_privilege : (%d)", ret);
            Throw(Exception::SetPrivilegeFailed);
        }
    }
    Catch(Exception::Base)
    {
        destroyAppInfoHandle(handle);
        destroyPkgInfoHandle(pkg_handle);
        return false;
    }

    destroyAppInfoHandle(handle);
    destroyPkgInfoHandle(pkg_handle);
    return true;
}

std::string SecuritySupport::getPluginProcessSoftLinkPath(const std::string& tzAppId)
{
    const std::string npruntimePostfix = ".npruntime";

    pkgmgrinfo_appinfo_h handle = NULL;
    std::string appBinPath;

    Try
    {
        getAppInfo(tzAppId, &handle);
        char* path = getExePath(handle);
        appBinPath = path;
    }
    Catch(Exception::Base)
    {
        destroyAppInfoHandle(handle);
        return "";
    }

    destroyAppInfoHandle(handle);
    return appBinPath + npruntimePostfix;
}
} // ClientModule
