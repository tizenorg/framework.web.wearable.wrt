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
 * @file    view_logic_web_notification_permission_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @brief   Implementation file of web Notification permission API
 */

#include "view_logic_web_notification_permission_support.h"

#include <string>
#include <dpl/log/secure_log.h>
#include <dpl/availability.h>
#include <wrt-commons/security-origin-dao/security_origin_dao_types.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <common/view_logic_help_popup_support.h>
#include <common/view_logic_security_origin_support_util.h>

#include <app.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <widget_string.h>

namespace ViewModule {
namespace WebNotificationPermissionSupport {
using namespace SecurityOriginDB;
using namespace ViewModule::SecurityOriginSupportUtil;

namespace {
// Function declare
bool askUserPermission(Evas_Object* parent, PermissionData* data);
void setPermissionResult(PermissionData* permData, Result result);
static void popupCallback(void* data, Evas_Object* obj, void* eventInfo);
static void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo);

bool askUserPermission(Evas_Object* parent, PermissionData* data)
{
    _D("called");
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
        WrtText::replacePS({WRT_POP_WEB_NOTIFICATION_PERMISSION,
                           appname,
                           origin});
    Evas_Object* popup = createPopup(parent,
                                     body.c_str(),
                                     WRT_BODY_REMEMBER_PREFERENCE,
                                     popupCallback,
                                     eaKeyCallback,
                                     data);
    if (popup == NULL) {
        delete data;
        return false;
    } else {
        evas_object_show(popup);
    }
    return true;
}

void setPermissionResult(PermissionData* permData, Result result)
{
    Assert(permData);
    if (result != RESULT_UNKNOWN) {
        permData->m_originDao->setSecurityOriginData(permData->m_originData, result);
    }

    Ewk_Notification_Permission_Request* permissionRequest =
        static_cast<Ewk_Notification_Permission_Request*>(permData->m_data);
    Eina_Bool ret = (result == RESULT_ALLOW_ALWAYS || result == RESULT_ALLOW_ONCE) ? EINA_TRUE : EINA_FALSE;
    ewk_notification_permission_request_set(permissionRequest, ret);
}

void popupCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, getResult(obj));
    delete permData;

    Evas_Object* popup = getPopup(obj);
    if (isNeedHelpPopup(popup)) {
        ViewModule::HelpPopupSupport::showClearDefaultPopup(popup);
    }
    evas_object_hide(popup);
    evas_object_del(popup);
}

void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, RESULT_DENY_ONCE);
    delete permData;

    evas_object_hide(obj);
    evas_object_del(obj);
}
} // anonymous namespace

void permissionRequest(
    Evas_Object* parent,
    SecurityOriginDAO* securityOriginDAO,
    void* data)
{
    Assert(securityOriginDAO);
    Assert(data);
    Ewk_Notification_Permission_Request* request =
        static_cast<Ewk_Notification_Permission_Request*>(data);
    const Ewk_Security_Origin* ewkOrigin =
        ewk_notification_permission_request_origin_get(request);
    Assert(ewkOrigin);

    SecurityOriginData securityOriginData(
        WrtDB::FEATURE_WEB_NOTIFICATION,
        Origin(
            DPL::FromUTF8String(ewk_security_origin_protocol_get(ewkOrigin)),
            DPL::FromUTF8String(ewk_security_origin_host_get(ewkOrigin)),
            ewk_security_origin_port_get(ewkOrigin)));

    // check cache database
    Result result = securityOriginDAO->getResult(securityOriginData);
    if (RESULT_ALLOW_ONCE == result || RESULT_ALLOW_ALWAYS == result) {
        _D("allow");
        ewk_notification_permission_request_set(request, EINA_TRUE);
        return;
    } else if (RESULT_DENY_ONCE == result || RESULT_DENY_ALWAYS == result) {
        _D("deny");
        ewk_notification_permission_request_set(request, EINA_FALSE);
        return;
    }

    // ask to user
    PermissionData* permissionData =
        new PermissionData(securityOriginDAO,
                           securityOriginData,
                           request);

    // suspend notification
    ewk_notification_permission_request_suspend(request);
    if (!askUserPermission(parent, permissionData)) {
        _W("Fail to create user permission popup");
        ewk_notification_permission_request_set(request, RESULT_DENY_ONCE);
    }
    return;
}
} // namespace WebNotificationPermissionSupport
} //namespace ViewModule
