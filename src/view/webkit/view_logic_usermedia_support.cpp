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
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <wrt-commons/security-origin-dao/security_origin_dao_types.h>

#include <common/view_logic_security_origin_support_util.h>
#include <common/view_logic_get_parent_window_util.h>
#include <permission_popup_manager.h>
#include <widget_string.h>
#include <app.h>

#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

namespace ViewModule {
using namespace SecurityOriginDB;
using namespace ViewModule::SecurityOriginSupportUtil;
namespace {
const char* const USERMEDIA_USE_ASK_BODY =
    "This application wants to use your media";

// function declare
void askUserForUsermediaPermission(Evas_Object* window, PermissionData* data);
Evas_Object* getPopup(Evas_Object* button);
static void popupCallback(void* data, Evas_Object* obj, void* eventInfo);

void askUserForUsermediaPermission(Evas_Object* window, PermissionData* data)
{
    LogDebug("askUserForUsermediaPermission called");

    Evas_Object* parentWindow = PopupUtil::getParentWindow(window);
    Evas_Object* popup = elm_popup_add(parentWindow);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(popup, WRT_POP_USERMEDIA_PERMISSION);

    Evas_Object* lButton = elm_button_add(popup);
    elm_object_style_set(lButton, "popup");
    elm_object_text_set(lButton, WRT_OPT_DENY);
    elm_object_part_content_set(popup, "button1", lButton);
    evas_object_smart_callback_add(lButton, "clicked", popupCallback, data);

    Evas_Object* rButton = elm_button_add(popup);
    elm_object_style_set(rButton, "popup");
    elm_object_text_set(rButton, WRT_OPT_ALLOW);
    elm_object_part_content_set(popup, "button2", rButton);
    evas_object_smart_callback_add(rButton, "clicked", popupCallback, data);
    evas_object_show(popup);
    PermissionPopupManagerSingleton::Instance().registerPopup(window, popup);
}

Evas_Object* getPopup(Evas_Object* button)
{
    Assert(button);

    Evas_Object* popup = button;
    while (strcmp(elm_object_widget_type_get(popup), "elm_popup")) {
        popup = elm_object_parent_widget_get(popup);
        if (!popup) {
            return NULL;
        }
    }
    return popup;
}

void popupCallback(void* data, Evas_Object* obj, void* /*eventInfo*/)
{
    LogDebug("popupCallback");
    Assert(data);
    PermissionData* permData = static_cast<PermissionData*>(data);
    Ewk_User_Media_Permission_Request* permissionRequest =
        static_cast<Ewk_User_Media_Permission_Request*>(permData->m_data);

    Assert(obj);
    Evas_Object* popup = getPopup(obj);
    Assert(popup);

    bool allow = !strcmp(WRT_OPT_ALLOW, elm_object_text_get(obj));
    Eina_Bool ret = allow ? EINA_TRUE : EINA_FALSE;
    ewk_user_media_permission_request_set(permissionRequest, ret);
    delete permData;
    evas_object_hide(popup);
    evas_object_del(popup);
}
} // namespace


bool hasPrivilege(DPL::String tizenId)
{
    bool ret = FALSE;

    WrtDB::WidgetDAOReadOnly widgetDao(tizenId);
    std::list<DPL::String> widgetPrivilege = widgetDao.getWidgetPrivilege();
    FOREACH(it, widgetPrivilege) {
        if (!DPL::ToUTF8String(*it).compare("http://tizen.org/privilege/mediacapture")) {
            LogDebug("Find privilege.");
            ret = TRUE;
        }
    }

    return ret;
}

void UsermediaSupport::usermediaPermissionRequest(Evas_Object* window,
                                                  SecurityOriginDAO* securityOriginDAO,
                                                  void* data,
                                                  DPL::String tizenId)
{
    LogDebug("usermediaPermissionRequest called");
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
        ewk_user_media_permission_request_set(permissionRequest, EINA_TRUE);
        return;
    } else if (RESULT_DENY_ONCE == result || RESULT_DENY_ALWAYS == result) {
        LogDebug("deny");
        ewk_user_media_permission_request_set(permissionRequest, EINA_FALSE);
        return;
    }

    // ask to user
    PermissionData* permissionData =
        new PermissionData(securityOriginDAO,
                           securityOriginData,
                           data);

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
