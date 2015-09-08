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
/**
 * @file    view_logic_usermedia_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "view_logic_usermedia_support.h"

#include <string>

#include <dpl/assert.h>
#include <dpl/log/log.h>
#include <dpl/availability.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <wrt-commons/security-origin-dao/security_origin_dao_types.h>
#include <common/view_logic_security_origin_support_util.h>
#include <common/view_logic_get_parent_window_util.h>
#include <widget_string.h>
#include <app.h>

#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

namespace ViewModule {
using namespace SecurityOriginDB;
using namespace ViewModule::SecurityOriginSupportUtil;
namespace {
// function declare
void askUserForUsermediaPermission(Evas_Object* window, PermissionData* data);
static void popupCallback(void* data, Evas_Object* obj, void* eventInfo);
void setPermissionResult(PermissionData* permData, Result result);
static void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo);

void askUserForUsermediaPermission(Evas_Object* window, PermissionData* data)
{
    LogDebug("called");
    std::string origin = DPL::ToUTF8String(data->m_originData.origin.host);
    if (origin.empty()) {
        origin = "local";
    }
    std::string appname;
    char* name = NULL;
    if (app_get_name(&name) == APP_ERROR_NONE) {
        appname = name;
        free(name);
    } else {
        appname = "application";
    }

    std::string body =
        WrtText::replacePS({WRT_POP_USERMEDIA_PERMISSION, appname, origin});

    Evas_Object* popup = createPopup(window,
                                     body.c_str(),
                                     WRT_BODY_REMEMBER_PREFERENCE,
                                     popupCallback,
                                     eaKeyCallback,
                                     data);

    if (popup == NULL) {
        LogError("Fail to create popup object");
        delete data;
        return;
    } else {
        evas_object_show(popup);
    }
}

void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    LogDebug("called");
    DPL_UNUSED_PARAM(eventInfo);

    Assert(data);
    Assert(obj);

    PermissionData* permData = static_cast<PermissionData*>(data);
    Evas_Object* popup = getPopup(obj);
    setPermissionResult(permData, RESULT_DENY_ONCE);

    delete permData;
    evas_object_hide(popup);
    evas_object_del(popup);
}

void popupCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    LogDebug("called");

    Assert(obj);
    Assert(data);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, getResult(obj));
    delete permData;

    Evas_Object* popup = getPopup(obj);

    evas_object_hide(popup);
    evas_object_del(popup);
}

void setPermissionResult(PermissionData* permData, Result result)
{
    Assert(permData);
    if (result != RESULT_UNKNOWN) {
        permData->m_originDao->setSecurityOriginData(permData->m_originData, result);
    }

    Ewk_User_Media_Permission_Request* permissionRequest =
        static_cast<Ewk_User_Media_Permission_Request*>(permData->m_data);

    Eina_Bool ret = (result == RESULT_ALLOW_ALWAYS || result == RESULT_ALLOW_ONCE) ? EINA_TRUE : EINA_FALSE;

    ewk_user_media_permission_reply(permissionRequest, ret);
}
} // namespace


bool hasPrivilege(DPL::String tizenId)
{
    bool ret = false;

    WrtDB::WidgetDAOReadOnly widgetDao(tizenId);
    std::list<DPL::String> widgetPrivilege = widgetDao.getWidgetPrivilege();
    FOREACH(it, widgetPrivilege) {
        if (!DPL::ToUTF8String(*it).compare("http://tizen.org/privilege/mediacapture")) {
            LogDebug("Find privilege.");
            ret = true;
        }
    }

    return ret;
}

void UsermediaSupport::usermediaPermissionRequest(Evas_Object* window,
                                                  SecurityOriginDAO* securityOriginDAO,
                                                  void* data,
                                                  DPL::String tizenId)
{
    LogDebug("called");
    Assert(securityOriginDAO);
    Assert(data);

    Ewk_User_Media_Permission_Request* permissionRequest =
        static_cast<Ewk_User_Media_Permission_Request*>(data);

    const Ewk_Security_Origin* ewkOrigin =
        ewk_user_media_permission_request_origin_get(permissionRequest);
    Assert(ewkOrigin);

    SecurityOriginData securityOriginData(
        WrtDB::FEATURE_USER_MEDIA,
        Origin(
            DPL::FromUTF8String(ewk_security_origin_protocol_get(ewkOrigin)),
            DPL::FromUTF8String(ewk_security_origin_host_get(ewkOrigin)),
            ewk_security_origin_port_get(ewkOrigin)));

    // In case of usermedia ewk doesn't support origin data
    // cache data also only store allow data by privilege
    Result result = securityOriginDAO->getResult(securityOriginData);
    if (RESULT_ALLOW_ONCE == result || RESULT_ALLOW_ALWAYS == result) {
        LogDebug("allow");
        ewk_user_media_permission_reply(permissionRequest, EINA_TRUE);
        return;
    } else if (RESULT_DENY_ONCE == result || RESULT_DENY_ALWAYS == result) {
        LogDebug("deny");
        ewk_user_media_permission_reply(permissionRequest, EINA_FALSE);
        return;
    }

    // ask to user
    PermissionData* permissionData =
        new PermissionData(securityOriginDAO,
                           securityOriginData,
                           permissionRequest);

    // check recorder privilege is exist or not. if privilege is not exist, set to deny without popup
    if (!hasPrivilege(tizenId)) {
        LogDebug("App no privilege. set to deny");

        permissionData->m_originDao->setSecurityOriginData(permissionData->m_originData, RESULT_DENY_ONCE);

        Ewk_User_Media_Permission_Request* permissionRequest =
            static_cast<Ewk_User_Media_Permission_Request*>(permissionData->m_data);

        ewk_user_media_permission_request_set(permissionRequest, EINA_FALSE);

        delete permissionData;
        return;
    }

    ewk_user_media_permission_request_suspend(permissionRequest);

    askUserForUsermediaPermission(window, permissionData);
    return;
}
} // namespace ViewModule
