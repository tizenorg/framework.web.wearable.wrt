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
/*
 * @file       view_logic_user_agent_support.cpp
 * @author     Jihoon Chung (jihoon.chung@samsung.com)
 * @version    1.0
 */

#include "view_logic_user_agent_support.h"

#include <string>
#include <cstring>
#include <dpl/platform.h>
#include <dpl/log/secure_log.h>
#include <dpl/string.h>
#include <widget_model.h>

#include <app.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace ViewModule {
namespace {
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
void setCustomUA(std::string customUA);
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
bool setAppInfo(WidgetModel* model, Evas_Object* wkView);
void printUA(Evas_Object* wkView);

#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
bool setCustomUA(WidgetModel* model, Evas_Object* wkView)
{
    std::string customUserAgent = model->SettingList.Get().getUserAgent();
    if (customUserAgent.empty()) {
        return false;
    } else {
        ewk_view_user_agent_set(wkView, customUserAgent.c_str());
        return true;
    }
    return false;
}
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)

bool setAppInfo(WidgetModel* model, Evas_Object* wkView)
{
    std::string appInfo; // appname/appversion
    char* name = NULL;
    if (app_get_name(&name) == APP_ERROR_NONE) {
        appInfo = name;
        free(name);
    } else {
        _W("Fail to get app name");
        if (name) {
            free(name);
        }
        return false;
    }

    DPL::OptionalString version = model->Version.Get();
    if (!!version) {
        std::string versionStr = DPL::ToUTF8String(*version);
        if (versionStr.empty()) {
            // version is empty
            // skip to set version field
        } else {
            appInfo += "/";
            appInfo += versionStr;
        }
    }
    if (ewk_view_application_name_for_user_agent_set(wkView, appInfo.c_str())
        == EINA_TRUE)
    {
        // verify
        const char* info =
            ewk_view_application_name_for_user_agent_get(wkView);
        if (!info || !strlen(info) || appInfo != info) {
            _W("Fail to verify app info in the UA");
            return false;
        }
        return true;
    } else {
        _W("Fail to set app info to UA");
        return false;
    }
    return true;
}

void printUA(Evas_Object* wkView)
{
    const char* ua = ewk_view_user_agent_get(wkView);
    _D("%s", ua);
}
} // anonymous namespace

void UserAgentSupport::setUserAgent(WidgetModel* model, Evas_Object* wkView)
{
    Assert(model);
    Assert(wkView);

#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
    // set custom UA
    if (setCustomUA(model, wkView)) {
        printUA(wkView);
        return;
    }
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)

    // In case of default UA, add appname/appversion
    if (setAppInfo(model, wkView)) {
        printUA(wkView);
    } else {
        // default UA
        printUA(wkView);
    }
    return;
}

} // ViewModule
