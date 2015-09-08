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
 * @file    view_logic_geolocation_support.cpp
 * @author  Grzegorz Krawczyk (g.krawczyk@samsung.com)
 */

#include "view_logic_geolocation_support.h"

#include <string>
#include <sstream>

#include <dpl/assert.h>
#include <dpl/log/log.h>
#include <dpl/log/secure_log.h>
#include <dpl/availability.h>
#include <wrt-commons/security-origin-dao/security_origin_dao_types.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <widget_string.h>
#include <common/view_logic_help_popup_support.h>
#include <common/view_logic_security_origin_support_util.h>
#include <Elementary.h>
#include <app.h>
#include <privacy_manager_client.h>
#include <privacy_manager_client_types.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

#define LOCATION_PRIVACY "http://tizen.org/privilege/location"

namespace ViewModule {
namespace GeolocationSupport {
using namespace SecurityOriginDB;
using namespace ViewModule::SecurityOriginSupportUtil;

namespace {
// function declare
void askUserForGeolocationPermission(
    Evas_Object* window,
    PermissionData* data);
void setPermissionResult(PermissionData* permData, Result result);
static void popupCallback(void* data, Evas_Object* obj, void* eventInfo);
static void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo);

void askUserForGeolocationPermission(
    Evas_Object* window,
    PermissionData* data)
{
    LogDebug("askUserForGeolocationPermission called");
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
        WrtText::replacePS({WRT_POP_GEOLOCATION_PERMISSION, appname, origin});
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

void setPermissionResult(PermissionData* permData, Result result)
{
    Assert(permData);
    if (result != RESULT_UNKNOWN) {
        permData->m_originDao->setSecurityOriginData(permData->m_originData, result);
    }

    Ewk_Geolocation_Permission_Request* permissionRequest =
        static_cast<Ewk_Geolocation_Permission_Request*>(permData->m_data);
    Eina_Bool ret = (result == RESULT_ALLOW_ALWAYS || result == RESULT_ALLOW_ONCE) ? EINA_TRUE : EINA_FALSE;

    std::string package_id = DPL::ToUTF8String(permData->m_pkgId);
    const char *privacy_list[] = {
      LOCATION_PRIVACY,
      "\0"
    };

    LogDebug("packageId " << package_id.c_str());
    int privacy_ret = privacy_manager_client_install_privacy(package_id.c_str(), privacy_list, false);
    LogDebug("privacy manager ret val: " << privacy_ret);

    ewk_geolocation_permission_reply(permissionRequest, ret);
}

void popupCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(obj);
    Assert(data);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, getResult(obj));
    delete permData;

    Evas_Object* popup = getPopup(obj);

// disable help popup in the Wearable.
// application manager is not exist in Wearble Setting App
#if 0
    if (isNeedHelpPopup(popup)) {
        ViewModule::HelpPopupSupport::showClearDefaultPopup(popup);
    }
#endif
    evas_object_hide(popup);
    evas_object_del(popup);
}

void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");
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
} // namespace


bool hasPrivilege(DPL::String tizenId)
{
    bool ret = false;

    WrtDB::WidgetDAOReadOnly widgetDao(tizenId);
    std::list<DPL::String> widgetPrivilege = widgetDao.getWidgetPrivilege();
    FOREACH(it, widgetPrivilege) {
        if (!DPL::ToUTF8String(*it).compare("http://tizen.org/privilege/location")) {
            LogDebug("Find privilege.");
            ret = true;
        }
    }

    return ret;
}

void geolocationPermissionRequest(
    Evas_Object* window,
    SecurityOriginDB::SecurityOriginDAO* securityOriginDAO,
    void* data,
    DPL::String tizenId)
{
    LogDebug("called");

    Assert(securityOriginDAO);
    Assert(data);
    GeoLocationData *geoData = static_cast<GeoLocationData*>(data);
    Ewk_Geolocation_Permission_Request* permissionRequest =
        static_cast<Ewk_Geolocation_Permission_Request*>(geoData->permissionRequest);

    const Ewk_Security_Origin* ewkOrigin =
        ewk_geolocation_permission_request_origin_get(
            permissionRequest);
    Assert(ewkOrigin);

    SecurityOriginData securityOriginData(
        WrtDB::FEATURE_GEOLOCATION,
        Origin(
            DPL::FromUTF8String(ewk_security_origin_protocol_get(ewkOrigin)),
            DPL::FromUTF8String(ewk_security_origin_host_get(ewkOrigin)),
            ewk_security_origin_port_get(ewkOrigin)));

    // check cache database
    Result result = securityOriginDAO->getResult(securityOriginData);
    if (RESULT_ALLOW_ONCE == result || RESULT_ALLOW_ALWAYS == result) {
        LogDebug("allow");
        ewk_geolocation_permission_reply(permissionRequest, EINA_TRUE);
        return;
    } else if (RESULT_DENY_ONCE == result || RESULT_DENY_ALWAYS == result) {
        LogDebug("deny");
        ewk_geolocation_permission_reply(permissionRequest, EINA_FALSE);
        return;
    }

    DPL::String pkgId = geoData->pkgId;

    // ask to user
    PermissionData* permissionData =
        new PermissionData(securityOriginDAO,
                           securityOriginData,
                           permissionRequest,
                           pkgId);

    // check whether geolocation privilege is exist or not.
    // if privilege is not exist, set to deny without popup
    if (!hasPrivilege(tizenId)) {
        LogDebug("App no privilege. set to deny");
        setPermissionResult(permissionData, RESULT_DENY_ONCE);
        delete permissionData;
        return;
    }

    // suspend geolocation
    ewk_geolocation_permission_request_suspend(permissionRequest);

    askUserForGeolocationPermission(window, permissionData);
    return;
}
} // namespace GeolocationSupport
} // namespace ViewModule
