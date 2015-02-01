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
 * @file    view_logic_web_storage_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "view_logic_web_storage_support.h"

#include <string>
#include <sstream>
#include <dpl/log/secure_log.h>
#include <dpl/availability.h>
#include <dpl/assert.h>
#include <wrt-commons/security-origin-dao/security_origin_dao_types.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <common/view_logic_help_popup_support.h>
#include <common/view_logic_security_origin_support_util.h>
#include <Elementary.h>
#include <widget_string.h>

namespace ViewModule {
using namespace SecurityOriginDB;
using namespace ViewModule::SecurityOriginSupportUtil;

namespace {
struct WebStoragePermissionData {
    WebStorageSupport::ewkQuotaReply m_replyEAPI;
    Evas_Object* m_ewkView;
    WebStoragePermissionData(WebStorageSupport::ewkQuotaReply replyEAPI,
                             Evas_Object* ewkView) :
        m_replyEAPI(replyEAPI),
        m_ewkView(ewkView)
    {}
};

// function declare
void askUserForWebStorageCreatePermission(
    Evas_Object* window,
    PermissionData* data);
void setPermissionResult(PermissionData* permData, Result result);
static void popupCallback(void* data, Evas_Object* obj, void* eventInfo);
static void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo);

void askUserForWebStorageCreatePermission(
    Evas_Object* window,
    PermissionData* data)
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
        WrtText::replacePS({WRT_POP_WEB_STORAGE_PERMISSION, appname, origin});
    Evas_Object* popup = createPopup(window,
                                     body.c_str(),
                                     WRT_BODY_REMEMBER_PREFERENCE,
                                     popupCallback,
                                     eaKeyCallback,
                                     data);

    if (popup == NULL) {
        _E("Fail to create popup object");
        delete data;
        return;
    } else {
        evas_object_show(popup);
    }
}

void setPermissionResult(PermissionData* permData, Result result)
{
    Assert(permData);
    WebStoragePermissionData* webStoragePermissionData =
        static_cast<WebStoragePermissionData*>(permData->m_data);

    if (result != RESULT_UNKNOWN) {
        permData->m_originDao->setSecurityOriginData(permData->m_originData, result);
    }
    Eina_Bool ret = (result == RESULT_ALLOW_ALWAYS || result == RESULT_ALLOW_ONCE) ? EINA_TRUE : EINA_FALSE;
    webStoragePermissionData->m_replyEAPI(webStoragePermissionData->m_ewkView, ret);
}

void popupCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, getResult(obj));
    WebStoragePermissionData* webStoragePermissionData =
        static_cast<WebStoragePermissionData*>(permData->m_data);
    delete webStoragePermissionData;
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

    WebStoragePermissionData* webStoragePermissionData =
        static_cast<WebStoragePermissionData*>(permData->m_data);
    delete webStoragePermissionData;
    delete permData;

    evas_object_hide(obj);
    evas_object_del(obj);
}
} // namespace

void WebStorageSupport::createPermissionRequest(
    Evas_Object* window,
    SecurityOriginDB::SecurityOriginDAO* securityOriginDAO,
    Evas_Object* ewkView,
    Ewk_Security_Origin* ewkOrigin,
    ewkQuotaReply replyEAPI)
{
    _D("called");
    Assert(securityOriginDAO);
    Assert(ewkOrigin);
    Assert(ewkView);
    SecurityOriginData securityOriginData(
        WrtDB::FEATURE_WEB_DATABASE,
        Origin(
            DPL::FromUTF8String(ewk_security_origin_protocol_get(ewkOrigin)),
            DPL::FromUTF8String(ewk_security_origin_host_get(ewkOrigin)),
            ewk_security_origin_port_get(ewkOrigin)));

    // check cache database
    Result result = securityOriginDAO->getResult(securityOriginData);
    if (result != RESULT_UNKNOWN) {
        Eina_Bool ret =
            (result == RESULT_ALLOW_ALWAYS || result == RESULT_ALLOW_ONCE) ?
                EINA_TRUE : EINA_FALSE;
        replyEAPI(ewkView, ret);
        return;
    }

    // ask to user
    WebStoragePermissionData* webStoragePermissionData =
        new WebStoragePermissionData(replyEAPI, ewkView);
    PermissionData* permissionData =
        new PermissionData(securityOriginDAO,
                           securityOriginData,
                           webStoragePermissionData);
    askUserForWebStorageCreatePermission(window, permissionData);
    return;
}
} // namespace ViewModule
