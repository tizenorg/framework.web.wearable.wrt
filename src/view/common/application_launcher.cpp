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
 * @file    application_launcher.cpp
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Implementation file for application launcher
 */

#include "application_launcher.h"

#include <dpl/log/secure_log.h>
#include <dpl/singleton_impl.h>

#include <app.h>
#include <app_manager.h>
#include <appsvc.h>
#include <notification.h>
#include <widget_string.h>

IMPLEMENT_SINGLETON(ApplicationLauncher)

namespace {
const char * const APP_CONTROL_EXTRA_DATA_KEY_PATH = "path";
const char * const APP_CONTROL_EXTRA_DATA_KEY_COOKIE = "cookie";
const char * const APP_CONTROL_EXTRA_DATA_KEY_MODE = "mode";
const char * const APP_CONTROL_EXTRA_DATA_VALUE_SLIENT = "silent";

const char * const SCHEME_TYPE_RTSP = "rtsp";
const char * const SCHEME_TYPE_HTML5_VIDEO = "html5video";
}

ApplicationLauncher::ApplicationLauncher() :
    m_windowHandle(0)
{
}

ApplicationLauncher::~ApplicationLauncher()
{}

void ApplicationLauncher::OnEventReceived(
    const ApplicationLauncherEvents::LaunchApplicationByAppService &event)
{
    int result;
    app_control_h app_control = event.GetArg0();
    app_control_reply_cb responseCallback = event.GetArg1();
    void *userData = event.GetArg2();

    if (m_windowHandle) {
        app_control_set_window(app_control, m_windowHandle);
    }

    result = app_control_send_launch_request(app_control, responseCallback, userData);
    if (result != APP_CONTROL_ERROR_NONE) {
        app_control_destroy(app_control);
        _E("Failed to run app_control : %d", result);
        return;
    }
    app_control_destroy(app_control);
    _D("Success to run app_control");
}

void ApplicationLauncher::OnEventReceived(
    const ApplicationLauncherEvents::LaunchApplicationByPkgname &event)
{
    using namespace ApplicationLauncherPkgname;
    std::string pkgName(event.GetArg0());

    if (PKG_NAME_DOWNLOAD_PROVIDER == pkgName) {
        std::string url(event.GetArg1());
        // This value needs for checking video, music contents later.
        //std::string mime_type(event.GetArg2());
        std::string cookie(event.GetArg3());

        if ("null" == url) {
            _E("url is empty");
            return;
        }

        app_control_h app_control = NULL;
        int ret = APP_CONTROL_ERROR_NONE;

        // create app_control
        ret = app_control_create(&app_control);
        if (APP_CONTROL_ERROR_NONE != ret && NULL == app_control) {
            _E("Fail to create app_control");
            return;
        }

        // set app_control operation
        ret = app_control_set_operation(app_control, APP_CONTROL_OPERATION_DOWNLOAD);
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to set operation [%d]", ret);
            app_control_destroy(app_control);
            return;
        }

        // set app_control uri
        ret = app_control_set_uri(app_control, url.c_str());
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to set uri [%d]", ret);
            app_control_destroy(app_control);
            return;
        }

        // set cookie
        if (cookie != "null") {
            ret = app_control_add_extra_data(app_control,
                                         APP_CONTROL_EXTRA_DATA_KEY_COOKIE,
                                         cookie.c_str());
            if (APP_CONTROL_ERROR_NONE != ret) {
                _D("Fail to add cookie [%d]", ret);
                app_control_destroy(app_control);
                return;
            }
        }

        ret = app_control_add_extra_data(app_control,
                                     APP_CONTROL_EXTRA_DATA_KEY_MODE,
                                     APP_CONTROL_EXTRA_DATA_VALUE_SLIENT);
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to set app_control extra data [%d]", ret);
            app_control_destroy(app_control);
            return;
        }

        if (m_windowHandle) {
            app_control_set_window(app_control, m_windowHandle);
        }

        //launch app_control
        ret = app_control_send_launch_request(app_control, NULL, NULL);
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to launch app_control [%d]", ret);
            app_control_destroy(app_control);
            return;
        }
        app_control_destroy(app_control);
        notification_status_message_post(WRT_POP_STARTING_DOWNLOADING);
        return;
    } else if (PKG_NAME_VIDEO_PLAYER == pkgName) {
        bool isRunning = false;
        if (APP_MANAGER_ERROR_NONE !=
            app_manager_is_running(PKG_NAME_VT_MAIN.c_str(), &isRunning))
        {
            _E("Fail to get app running information");
            return;
        }
        if (true == isRunning) {
            _E("video-call is running");
            return;
        }

        std::string scheme(event.GetArg1());
        std::string uri(event.GetArg2());
        std::string cookie(event.GetArg3());
        const char* url;

        if ("null" == scheme) {
            _E("scheme is empty");
            return;
        }
        if ("null" == uri) {
            _E("uri is empty");
            return;
        }

        if (SCHEME_TYPE_RTSP == scheme ||
            SCHEME_TYPE_HTML5_VIDEO == scheme)
        {
            url = uri.c_str();
        } else {
            _E("scheme is invalid!!");
            return;
        }

        app_control_h app_control = NULL;
        int ret = APP_CONTROL_ERROR_NONE;

        // create app_control
        ret = app_control_create(&app_control);
        if (APP_CONTROL_ERROR_NONE != ret && NULL == app_control) {
            _E("Fail to create app_control");
            return;
        }

        // set url
        if (!url || strlen(url) == 0) {
            _E("Fail to get url");
            app_control_destroy(app_control);
            return;
        }
        ret = app_control_add_extra_data(app_control,
                                     APP_CONTROL_EXTRA_DATA_KEY_PATH,
                                     url);
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to set url [%d]", ret);
            app_control_destroy(app_control);
            return;
        }

        // set cookie
        if (SCHEME_TYPE_HTML5_VIDEO == scheme) {
            if ("null" != cookie) {
                ret = app_control_add_extra_data(app_control,
                                             APP_CONTROL_EXTRA_DATA_KEY_COOKIE,
                                             cookie.c_str());
                if (APP_CONTROL_ERROR_NONE != ret) {
                    _E("Fail to add cookie [%d]", ret);
                    app_control_destroy(app_control);
                    return;
                }
            }
        }

        // set package
        ret = app_control_set_app_id(app_control, PKG_NAME_VIDEO_PLAYER.c_str());
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to set package app_control [%d]", ret);
            app_control_destroy(app_control);
            return;
        }

        // set window handle when available
        if (m_windowHandle) {
            app_control_set_window(app_control, m_windowHandle);
        }

        //launch app_control
        ret = app_control_send_launch_request(app_control, NULL, NULL);
        if (APP_CONTROL_ERROR_NONE != ret) {
            _E("Fail to launch app_control [%d]", ret);
            app_control_destroy(app_control);
            return;
        }
        app_control_destroy(app_control);
        return;
    } else {
        _E("Not implemented application : %s", pkgName.c_str());
    }

    _D("Success to launch application : %s", pkgName.c_str());
    return;
}

void ApplicationLauncher::setWidgetTizenId(const std::string& tizenId)
{
    m_tizenId = tizenId;
}

void ApplicationLauncher::setWindowHandle(unsigned windowHandle)
{
    m_windowHandle = windowHandle;
}
