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
 * @file    view_logic.cpp
 * @author  Pawel Sikorski (p.sikorsk@samsung.com)
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @brief   View logic for Webkit2
 */
#include "view_logic.h"

#include <cstring>
#include <string>
#include <dpl/assert.h>
#include <dpl/log/log.h>
#include <dpl/log/secure_log.h>
#include <dpl/optional_typedefs.h>
#include <dpl/string.h>
#include <dpl/foreach.h>
#include <dpl/availability.h>
#include <dpl/platform.h>

#include <Elementary.h>
#include <efl_assist.h>
#include <pcrecpp.h>
#include <dd-deviced.h>
#include <widget_model.h>
#include <system_settings.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/utils/wrt_global_settings.h>

#include <application_data.h>
#include <common/application_launcher.h>
#include <common/message_support.h>
#include <common/scheme.h>
#include <common/view_logic_apps_support.h>
#include <common/view_logic_certificate_support.h>
#include <common/view_logic_custom_header_support.h>
#include <common/view_logic_privilege_support.h>
#include <common/view_logic_security_support.h>
#include <common/view_logic_security_origin_support.h>
#include <common/view_logic_storage_support.h>
#include <common/view_logic_uri_support.h>
#include <common/view_logic_vibration_support.h>
#include <view_logic_authentication_request_support.h>
#include <view_logic_certificate_confirm_support.h>
#include <view_logic_geolocation_support.h>
#include <view_logic_message_support.h>
#ifdef ORIENTATION_ENABLED
#include <view_logic_orientation_support.h>
#endif
#include <view_logic_scheme_support.h>
#include <view_logic_usermedia_support.h>
#include <view_logic_web_notification_data.h>
#include <view_logic_web_notification_support.h>
#include <view_logic_web_notification_permission_support.h>
#include <view_logic_web_storage_support.h>

#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <dpl/localization/w3c_file_localization.h>
#include <js_overlay_types.h>
#include <dispatch_event_support.h>
#include <i_runnable_widget_object.h>
#include <profiling_util.h>
#include <wrt-commons/custom-handler-dao-ro/CustomHandlerDatabase.h>
#include <wrt-commons/custom-handler-dao-rw/custom_handler_dao.h>
#include <popup-runner/PopupInvoker.h>
#include <plugins-ipc-message/ipc_message_support.h>
#include <appsvc.h>

namespace {
// IME State value
const char * const IME_STATE_ON = "on";
const char * const IME_STATE_OFF = "off";

const char PROTOCOL_HANDLER_ASK_MSG[] = "Add protocol?";
const char PROTOCOL_HANDLER_ASK_TITLE[] = "Add protocol";
const char PROTOCOL_HANDLER_ASK_REMEMBER[] = "Remember dicision";
const char CONTENT_HANDLER_ASK_MSG[] = "Add content?";
const char CONTENT_HANDLER_ASK_TITLE[] = "Add content";
const char CONTENT_HANDLER_AKS_REMEMBER[] = "Remember dicision";

const char* const DEFAULT_ENCODING = "UTF-8";
#if ENABLE(CONTENT_SECURITY_POLICY)
const char* const DEFAULT_CSP_POLICY =
    "default-src *; script-src 'self'; style-src 'self'; object-src 'none';";
#endif
// SCHEME
const char * const SCHEME_BOX_SLASH = "box://";
#ifdef ORIENTATION_ENABLED
const double ORIENTATION_THRESHOLD = 0.5;
#endif

// Ewk Smart Class
const char* EWK_SMART_CLASS_USER_DATA = "__sc_userdata__";
} // anonymous namespace

std::map<const std::string,
         const Evas_Smart_Cb> ViewLogic::m_ewkCallbacksMap = {
    { "load,started", &ViewLogic::loadStartedCallback },
    { "load,finished", &ViewLogic::loadFinishedCallback },
    { "load,progress,started", &ViewLogic::loadProgressStartedCallback },
    { "load,progress", &ViewLogic::loadProgressCallback },
    { "load,progress,finished", &ViewLogic::loadProgressFinishedCallback },
    { "webprocess,crashed", &ViewLogic::processCrashedCallback },
    // WKPagePolicyClient
    { "policy,navigation,decide", &ViewLogic::policyNavigationDecideCallback },
    { "policy,newwindow,decide", &ViewLogic::policyNewWindowDecideCallback },
    { "policy,response,decide", &ViewLogic::pageResponseDecideCallback },
    // WKPageContextMenuClient
    { "contextmenu,customize", &ViewLogic::contextmenuCustomizeCallback },
    // EWK Notification Callback
    { "notification,show", &ViewLogic::notificationShowCallback },
    { "notification,cancel", &ViewLogic::notificationCancelCallback },
    { "fullscreen,enterfullscreen", &ViewLogic::enterFullscreenCallback },
    { "fullscreen,exitfullscreen", &ViewLogic::exitFullscreenCallback },
    // IME Callback
    // when ime start to be showed on the webview,
    // this callback will be called
    { "inputmethod,changed", &ViewLogic::imeChangedCallback },
    // this callback will be called
    //  when ime finishes to be showed on the webview
    // "event_info" arg of this callback is always NULL point
    // if web content should know size of ime,
    //  use "inputmethod,changed" instead of this.
    //
    { "editorclient,ime,opened", &ViewLogic::imeOpenedCallback },
    // when ime finished to be hidden,
    // this callback will be called
    { "editorclient,ime,closed", &ViewLogic::imeClosedCallback },
    // Custom handlers
    { "protocolhandler,registration,requested",
      &ViewLogic::protocolHandlerRegistrationCallback },
    { "protocolhandler,isregistered",
      &ViewLogic::protocolHandlerIsRegisteredCallback },
    { "protocolhandler,unregistration,requested",
      &ViewLogic::protocolHandlerUnregistrationCallback },
    { "contenthandler,registration,requested",
      &ViewLogic::contentHandlerRegistrationCallback },
    { "contenthandler,isregistered",
      &ViewLogic::contentHandlerIsRegisteredCallback },
    { "contenthandler,unregistration,requested",
      &ViewLogic::contentHandlerUnregistrationCallback },
    { "request,certificate,confirm",
      &ViewLogic::certificateConfirmRequestCallback },
    { "authentication,request",
      &ViewLogic::authenticationRequestCallback },
    { "frame,rendered",
      &ViewLogic::viewFrameRenderedCallback },
#ifdef ORIENTATION_ENABLED
    { "mediacontrol,rotate,horizontal",
      &ViewLogic::mediacontrolRotateHorizontal },
    { "mediacontrol,rotate,vertical",
      &ViewLogic::mediacontrolRotateVertical },
    { "mediacontrol,rotate,exit",
      &ViewLogic::mediacontrolRotateExit },
#endif
    { "popup,reply,wait,start",
      &ViewLogic::popupReplyWaitStart },
    { "popup,reply,wait,finish",
      &ViewLogic::popupReplyWaitFinish },
    { "console,message",
      &ViewLogic::consoleMessageCallback },
#ifdef ORIENTATION_ENABLED
    { "rotate,prepared", &ViewLogic::rotatePreparedCallback },
#endif
    { "video,hwoverlay,enabled",
      &ViewLogic::enabledVideoHwOverlayCallback },
    { "video,hwoverlay,disabled",
      &ViewLogic::disabledVideoHwOverlayCallback },
    { "vibrate",
      &ViewLogic::vibrateCallback },
    { "cancel,vibration",
      &ViewLogic::cancelVibrationCallback },
};

ViewLogic::ViewLogic() :
    m_ewkContext(0),
    m_attachedToCustomHandlerDao(false),
    m_currentEwkView(0),
    m_closedEwkView(NULL),
    m_window(NULL),
    m_model(0),
    m_cbs(new WRT::UserDelegates),
    m_imeWidth(0),
    m_imeHeight(0),
    m_isBackgroundSupport(false),
#ifdef ORIENTATION_ENABLED
    m_rotateAngle(0),
    m_deferredRotateAngle(
        ViewModule::OrientationSupport::DEFERRED_ORIENTATION_EMPTY),
    m_orientationThresholdTimer(NULL),
#endif
    m_isPopupReplyWait(false),
    m_isFullscreenByPlatform(false),
    m_category(0),
    m_appsSupport(new ViewModule::AppsSupport()),
    m_vibrationSupport(new ViewModule::VibrationSupport()),
    m_webNotificationSupport(new ViewModule::WebNotificationSupport())
{
    ApplicationLauncherSingleton::Instance().Touch();
}

ViewLogic::~ViewLogic()
{
    detachFromCustomHandlersDao();
}

bool ViewLogic::createView(Ewk_Context* context, Evas_Object* window)
{
    LogDebug("enter");
    if (!context || !window) {
        return false;
    }

    // theme setting
    const char *theme = elm_theme_get(NULL);
    if (theme) {
        m_theme = theme;
        LogDebug("theme is " << m_theme);
    }

    // set members
    m_ewkContext = context;
    m_window = window;

    // TODO page group should be different per appid
    //      for support of multiple injected bundles on one web process
    m_pageGroup = ewk_page_group_create("");

    if (!createEwkView()) {
        return false;
    }

    return true;
}

void ViewLogic::prepareView(WidgetModel* m, const std::string &startUrl, int category)
{
    LogDebug("View prepare");

    Assert(m);
    m_model = m;
    m_startUrl = startUrl;
    Assert(NULL != m_ewkContext);
    Assert(m_window);

    m_category = category;

    ADD_PROFILING_POINT("initializeSupport", "start");
    initializeSupport();
    ADD_PROFILING_POINT("initializeSupport", "stop");
    ewkClientInit(m_currentEwkView);
    ADD_PROFILING_POINT("prepareEwkView", "start");
    prepareEwkView(m_currentEwkView);
    ADD_PROFILING_POINT("prepareEwkView", "stop");
    initializePluginLoading();
    initializeXwindowHandle();
}

struct _WidgetViewInfo{
    std::string url;
    Evas_Object_Event_Cb callback;
    Evas_Object* view;
    Ecore_Timer* timeout;
};

void ViewLogic::showWidget()
{
    LogDebug("showing widget");
    AssertMsg(NULL != m_currentEwkView, "ewk_view not created at this point");
    std::string url =
        ViewModule::UriSupport::getUri(m_model, m_startUrl);
    if (url.empty()) {
        LogError("Localized current URI doesn't exist");
        return;
    }
    LogDebug("url : " << url);

    //to delay ewk_view_url_set until resize callback was called
    auto callback = [](void* data, Evas* e, Evas_Object* obj, void* eventInfo) -> void {
        _WidgetViewInfo *info = static_cast<_WidgetViewInfo*>(data);
        ewk_view_url_set(info->view, info->url.c_str());
        evas_object_event_callback_del(info->view, EVAS_CALLBACK_RESIZE, info->callback);
        ecore_timer_del(info->timeout);
        delete info;
    };
    auto timeout = [](void* data) -> Eina_Bool {
        _WidgetViewInfo *info = static_cast<_WidgetViewInfo*>(data);
        ewk_view_url_set(info->view, info->url.c_str());
        evas_object_event_callback_del(info->view, EVAS_CALLBACK_RESIZE, info->callback);
        delete info;
        return ECORE_CALLBACK_CANCEL;
    };

    _WidgetViewInfo *info = new _WidgetViewInfo();
    info->url = url;
    info->callback = callback;
    info->view = m_currentEwkView;
    //resize callback was not called until after 0.1 sec, called by timer to prevent blocking
    info->timeout = ecore_timer_add(0.1, timeout, info);
    evas_object_event_callback_add(m_currentEwkView, EVAS_CALLBACK_RESIZE, callback, info);

    if (m_cbs->setWebviewCallback) {
        m_cbs->setWebviewCallback(m_currentEwkView);
    }
}

void ViewLogic::hideWidget()
{
    LogDebug("hiding widget");
    ViewModule::StorageSupport::deinitializeStorage(m_model);
    m_appsSupport->deinitialize();
    m_vibrationSupport->deinitialize();
    m_webNotificationSupport->deinitialize();
    system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_FONT_TYPE);

    while (!m_ewkViewList.empty()) {
        LogDebug("pop webview: " << m_ewkViewList.back());
        removeEwkView(m_ewkViewList.back());
    }
    m_ewkViewList.clear();
}

void ViewLogic::suspendWidget()
{
    LogDebug("Pausing widget");
    Assert(m_model);

    if (!m_currentEwkView) {
        LogWarning("Cannot suspend widget without view");
    } else {
        // In case of idle-clock, web view has to be visible to support home edit mode
        // and extra user scenario.
        if (m_category != 1) {
            setEwkViewInvisible(m_currentEwkView);
        }

        FOREACH(it, m_ewkViewList) {
            if (*it != m_currentEwkView) {
                suspendWebkit(*it);
            }
        }

        suspendWebkit(m_currentEwkView);
    }
}

void ViewLogic::resumeWidget()
{
    LogDebug("Resume widget");
    Assert(m_model);

    if (m_currentEwkView)
    {
        setEwkViewVisible(m_currentEwkView);

        FOREACH(it, m_ewkViewList) {
            if (*it != m_currentEwkView) {
                resumeWebkit(*it);
            }
        }

        resumeWebkit(m_currentEwkView);
    }
}

void ViewLogic::resetWidgetFromSuspend()
{
    LogDebug("Reset widget from suspend");
    Assert(m_currentEwkView);

    if (m_isPopupReplyWait) {
        setEwkViewVisible(m_currentEwkView);

        FOREACH(it, m_ewkViewList) {
            if (*it != m_currentEwkView) {
                resumeWebkit(*it);
            }
        }

        resumeWebkit(m_currentEwkView);
    }
#if ENABLE(JAVASCRIPT_ONBEFOREUNLOAD_EVENT)
    else {
        setEwkViewVisible(m_currentEwkView);

        FOREACH(it, m_ewkViewList) {
            if (*it != m_currentEwkView) {
                resumeWebkit(*it);
            }
        }

        resumeWebkit(m_currentEwkView);
    }
#endif
    resetWidgetCommon();
}

void ViewLogic::resetWidgetFromResume()
{
    LogDebug("Reset widget from resume");

    resetWidgetCommon();
}

void ViewLogic::resetWidgetCommon()
{
    Assert(m_currentEwkView);

    if (m_isPopupReplyWait) {
        // Current WebProcess is blocked by some reason, js alert.
        // This case "reset" event cannot pass to application(javascript layer).
        // Free bundle data to use next "reset" event.
        _D("Webkit is blocked. do resume");
        ApplicationDataSingleton::Instance().freeBundle();
    } else {
        bool isSelfTarget = false;
        std::string url = ViewModule::UriSupport::getUri(m_model, m_startUrl, &isSelfTarget);

        if (isSelfTarget) {
            ViewLogicMessageSupport::setCustomProperties(m_ewkContext, NULL,
                ApplicationDataSingleton::Instance().getEncodedBundle());
            DispatchEventSupport::dispatchAppControlEvent(m_currentEwkView);
        } else {
#if ENABLE(JAVASCRIPT_ONBEFOREUNLOAD_EVENT)
            initializePluginLoading();
            ewk_view_url_set(m_currentEwkView, url.c_str());
#else
            while (!m_ewkViewList.empty()) {
                removeEwkView(m_ewkViewList.back());
            }
            m_currentEwkView = NULL;

            createEwkView();
            ewkClientInit(m_currentEwkView);
            prepareEwkView(m_currentEwkView);

            initializePluginLoading();

            if (m_cbs->setWebviewCallback) {
                 m_cbs->setWebviewCallback(m_currentEwkView);
            }
            ewk_view_url_set(m_currentEwkView, url.c_str());
#endif
        }
    }

    elm_win_activate(m_window);
}

// keep this code to handle future requirement.
void ViewLogic::TimeTick(long time)
{
#if 0
    LogDebug("throw time tick custom event");

    std::stringstream script;
    script << "var __event = document.createEvent(\"CustomEvent\");\n"
           << "__event.initCustomEvent(\"timetick\", true, true);\n"
           << "document.dispatchEvent(__event);\n"
           << "\n"
           << "for (var i=0; i < window.frames.length; i++)\n"
           << "{ window.frames[i].document.dispatchEvent(__event); }";

    if (true)
    {
        LogDebug("script : " << script.str());
    }

    if (ewk_view_script_execute(m_currentEwkView, script.str().c_str(), NULL, NULL) != EINA_TRUE)
    {
        LogDebug("ewk_view_script_execute returned FALSE!");
    }
#endif
}

void ViewLogic::AmbientTick(long time)
{
    LogDebug("throw ambient time tick custom event");

    std::stringstream script;
    script << "var __event = document.createEvent(\"CustomEvent\");\n"
           << "__event.initCustomEvent(\"timetick\", true, true);\n"
           << "document.dispatchEvent(__event);\n"
           << "\n"
           << "for (var i=0; i < window.frames.length; i++)\n"
           << "{ window.frames[i].document.dispatchEvent(__event); }";

    if (true)
    {
        LogDebug("script : " << script.str());
    }

    if (ewk_view_script_execute(m_currentEwkView, script.str().c_str(), NULL, NULL) != EINA_TRUE)
    {
        LogDebug("ewk_view_script_execute returned FALSE!");
    }
}

void ViewLogic::AmbientModeChanged(bool ambient_mode)
{
    LogDebug("throw ambient mode change custom event");

    std::stringstream script;
    script << "var __event = document.createEvent(\"CustomEvent\");\n"
           << "var __detail = {};\n"
           << "__event.initCustomEvent(\"ambientmodechanged\", true, true, __detail);\n"
           << "__event.detail.ambientMode = " << (ambient_mode ? "true" : "false") << ";\n"
           << "document.dispatchEvent(__event);\n"
           << "\n"
           << "for (var i=0; i < window.frames.length; i++)\n"
           << "{ window.frames[i].document.dispatchEvent(__event); }";

    if (true)
    {
        LogDebug("script : " << script.str());
    }

    if (ewk_view_script_execute(m_currentEwkView, script.str().c_str(), NULL, NULL) != EINA_TRUE)
    {
        LogDebug("ewk_view_script_execute returned FALSE!");
    }
}


void ViewLogic::backward()
{
    if (ewk_view_back_possible(m_currentEwkView)) {
        ewk_view_back(m_currentEwkView);
    } else {
        if (1 >= m_ewkViewList.size()) {
            // If there is no previous page, widget move to backgroud.
            LogDebug("Widget move to backgroud");
            elm_win_lower(m_window);
        } else {
            // Back to previous webview
            LogDebug("Widget move to previous webview");
            m_closedEwkView = m_currentEwkView;
            ecore_idler_add(windowCloseIdlerCallback, this);
        }
    }
}

void ViewLogic::reloadStartPage()
{
    LogDebug("Reload Start Page");
    // set preferred languages
    std::string customHeaderString = ViewModule::CustomHeaderSupport::getValueByField(ViewModule::CustomHeaderSupport::ACCEPT_LANGUAGE);
    if (!customHeaderString.empty()) {
        LogDebug("preferred languages=[" << customHeaderString << "]");
        Eina_List* list = eina_list_append(NULL, customHeaderString.c_str());
        ewk_context_preferred_languages_set(list);
    }

    ewk_context_resource_cache_clear(m_ewkContext);
    ewk_view_reload_bypass_cache(m_currentEwkView);
}

Evas_Object* ViewLogic::getCurrentWebview()
{
    LogDebug("get current webview");
    return m_currentEwkView;
}

void ViewLogic::fireJavascriptEvent(int event, void* data)
{
    ViewLogicMessageSupport::dispatchJavaScriptEvent(
        m_ewkContext,
        static_cast<WrtPlugins::W3C::CustomEventType>(event),
        data);
}

void ViewLogic::setUserCallbacks(const WRT::UserDelegatesPtr& cbs)
{
    m_cbs = cbs;
}

void ViewLogic::checkSyncMessageFromBundle(
        const char* name,
        const char* /*body*/,
        char** returnData)
{
    LogDebug("didReceiveSynchronousMessage called");
    Assert(name);

    LogDebug("received : " << name);
    if (!strcmp(name, Message::TizenScheme::GET_WINDOW_HANDLE)) {
        if (m_window) {
            Ecore_X_Window handle = elm_win_xwindow_get(m_window);
            if (handle != 0) {
                std::stringstream ss;
                ss << handle;
                std::string ret  = ss.str();
                if (returnData) {
                    *returnData = strdup(ret.c_str());
                }
            } else {
                LogDebug("X window isn't exist");
            }
        }
    } else if (!strcmp(name, Message::ToUIProcess::BACKGROUND_SUPPORTED)) {
        if (m_isBackgroundSupport) {
            *returnData = strdup("true");
        } else {
            *returnData = strdup("false");
        }
    }
}

void ViewLogic::checkAsyncMessageFromBundle(const char* name, const char* body)
{
    Assert(name);
    _D("received : %s", name);

    if (!strcmp(name, Message::ToUIProcess::BLOCKED_URL)) {
        // Currently WebProcess informs obly about blocked
        // URI - URI localization and security chekcs are
        // done by WebProcess itself (see: wrt-injected-bundle.cpp
        // and bundle_uri_handling.cpp)
        std::string msgBody = (body) ? (body) : "";
        if (!msgBody.empty() && m_cbs->blockedUrlPolicyCallback) {
            m_cbs->blockedUrlPolicyCallback(msgBody);
            m_blockedUri = msgBody;
        }
    } else if (!strcmp(name, Message::TizenScheme::CLEAR_ALL_COOKIES)) {
        Ewk_Cookie_Manager* cookieManager =
            ewk_context_cookie_manager_get(m_ewkContext);
        if (!cookieManager) {
            _E("Fail to get cookieManager");
            return;
        }
        ewk_cookie_manager_cookies_clear(cookieManager);
    } else if (!strcmp(name, IPCMessageSupport::TIZEN_CHANGE_USERAGENT)) {
        std::string msgBody = (body) ? (body) : "";

        std::string strId = msgBody.substr(0, msgBody.find_first_of('_'));
        std::string strBody = msgBody.substr(msgBody.find_first_of('_')+1);
        _D("Id: %s , Body %s", strId.c_str(), strBody.c_str());

        ewk_view_user_agent_set(m_currentEwkView, strBody.c_str());
        _D("get UA: %s", ewk_view_user_agent_get(m_currentEwkView));

        IPCMessageSupport::replyAsyncMessageToWebProcess(m_ewkContext,
                                                         atoi(strId.c_str()),
                                                         "success");
        return;
    } else if (!strcmp(name, IPCMessageSupport::TIZEN_DELETE_ALL_COOKIES)) {
        std::string msgBody = (body) ? (body) : "";
        std::string strId = msgBody.substr(0, msgBody.find_first_of('_'));
        std::string strBody = msgBody.substr(msgBody.find_first_of('_')+1);

        Ewk_Cookie_Manager* cookieManager =
            ewk_context_cookie_manager_get(m_ewkContext);
        if (!cookieManager) {
            _E("Fail to get cookieManager");
            IPCMessageSupport::replyAsyncMessageToWebProcess(
                m_ewkContext,
                atoi(strId.c_str()),
                "error");
            return;
        }
        ewk_cookie_manager_cookies_clear(cookieManager);
        IPCMessageSupport::replyAsyncMessageToWebProcess(m_ewkContext,
                                                         atoi(strId.c_str()),
                                                         "success");
        return;
    } else if (!strcmp(name, IPCMessageSupport::TIZEN_EXIT) ||
               !strcmp(name, IPCMessageSupport::TIZEN_HIDE))
    {
        bool ret =
            ViewModule::SchemeSupport::HandleTizenScheme(name,
                                                         m_window,
                                                         m_currentEwkView);
        if (ret == false) {
            _E("Fail to handle tizen scheme %s", name);
        }
        // Not need to send reply
        return;
    } else {
        _W("Not defined message");
    }
}

void ViewLogic::downloadData(const char* url)
{
    LogDebug("enter");
    if (!url) {
        return;
    }
    m_appsSupport->downloadRequest(url, NULL, NULL);
}

void ViewLogic::activateVibration(bool on, uint64_t time)
{
    LogDebug("enter vibrationTime=" << time);
    if (on) {
        m_vibrationSupport->startVibration(static_cast<long>(time));
    } else {
        m_vibrationSupport->stopVibration();
    }
}

void ViewLogic::initializeSupport()
{
    // background support
    if (m_model->SettingList.Get().getBackgroundSupport()
        == BackgroundSupport_Enable)
    {
        LogDebug("Background support enabled, set process active");
        deviced_inform_active(getpid());
        m_isBackgroundSupport = true;
    }
    system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_FONT_TYPE,
                                   systemSettingsChangedCallback,
                                   this);

    ViewModule::StorageSupport::initializeStorage(m_model);
    m_appsSupport->initialize(m_model, elm_win_xwindow_get(m_window));
    m_securityOriginSupport.reset(new ViewModule::SecurityOriginSupport(m_model));
    m_certificateSupport.reset(new ViewModule::CertificateSupport(m_model));
    m_privilegeSupport.reset(new ViewModule::PrivilegeSupport(m_model));
    m_webNotificationSupport->initialize(m_model->TzPkgId.Get());
}

void ViewLogic::initializePluginLoading()
{
    // inform wrt information for plugin loading to web process
    ViewLogicMessageSupport::start(
        m_ewkContext,
        m_model->TizenId,
        elm_config_scale_get(),
        ApplicationDataSingleton::Instance().getEncodedBundle(),
        m_theme.c_str());
}

void ViewLogic::initializeXwindowHandle()
{
    if (m_window) {
        unsigned int handle = elm_win_xwindow_get(m_window);
        ViewLogicMessageSupport::setXwindowHandle(
            m_ewkContext,
            handle);
    }
}

void ViewLogic::ewkClientInit(Evas_Object *wkView)
{
    AssertMsg(NULL != wkView, "ewk_view not created at this point");

    FOREACH(it, m_ewkCallbacksMap)
    {
        evas_object_smart_callback_add(
            wkView,
            it->first.c_str(),
            it->second,
            this);
    }
    ewk_view_exceeded_database_quota_callback_set(
        wkView,
        exceededDatabaseQuotaCallback,
        this);
    ewk_view_exceeded_indexed_database_quota_callback_set(
        wkView,
        exceededIndexedDatabaseQuotaCallback,
        this);
    ewk_view_exceeded_local_file_system_quota_callback_set(
        wkView,
        exceededLocalFileSystemQuotaCallback,
        this);
    ewk_view_geolocation_permission_callback_set(
        wkView,
        geolocationPermissionRequestCallback,
        this);
    ewk_view_user_media_permission_callback_set(
        wkView,
        usermediaPermissionRequestCallback,
        this);
    ewk_view_notification_permission_callback_set(
        wkView,
        notificationPermissionRequestCallback,
        this);
    ea_object_event_callback_add(wkView,
                                 EA_CALLBACK_BACK,
                                 eaKeyCallback,
                                 this);
    ea_object_event_callback_add(wkView,
                                 EA_CALLBACK_MORE,
                                 eaKeyCallback,
                                 this);
    // Always register access object even application doesn't support
    // accessibility. In case of accessibility isn't supported, efl_assist
    // shows warning message by syspopup.
    // initScreenReaderSupport is related method. (window_data.cpp)
    elm_access_object_register(wkView, m_window);
    // add callback to handle rotary event
    eext_rotary_object_event_callback_add(wkView, rotaryCallback, this);
    eext_rotary_object_event_activated_set(wkView, EINA_TRUE);
}

void ViewLogic::ewkClientDeinit(Evas_Object *wkView)
{
    LogDebug("ewkClientDeinit");
    AssertMsg(NULL != wkView, "ewk_view not created at this point");

    FOREACH(it, m_ewkCallbacksMap)
    {
        evas_object_smart_callback_del(
            wkView,
            it->first.c_str(),
            it->second);
    }
    ewk_view_exceeded_database_quota_callback_set(wkView, NULL, NULL);
    ewk_view_exceeded_indexed_database_quota_callback_set(wkView, NULL, NULL);
    ewk_view_exceeded_local_file_system_quota_callback_set(wkView, NULL, NULL);
    ewk_view_geolocation_permission_callback_set(wkView, NULL, NULL);
    ewk_view_user_media_permission_callback_set(wkView, NULL, NULL);
    ewk_view_notification_permission_callback_set(wkView, NULL, NULL);
    ea_object_event_callback_del(wkView,
                                 EA_CALLBACK_BACK,
                                 eaKeyCallback);
    ea_object_event_callback_del(wkView,
                                 EA_CALLBACK_MORE,
                                 eaKeyCallback);
#ifdef ORIENTATION_ENABLED
    if (m_orientationThresholdTimer) {
        ecore_timer_del(m_orientationThresholdTimer);
        m_orientationThresholdTimer = NULL;
    }
#endif
    elm_access_object_unregister(wkView);
}

bool ViewLogic::createEwkView()
{
    LogDebug("createEwkView");
    ADD_PROFILING_POINT("ewk_view_add_with_context", "start");
    Evas_Object* newEwkView = addEwkView();
    ADD_PROFILING_POINT("ewk_view_add_with_context", "stop");

    if (!newEwkView) {
        LogError("View creation failed");
        Wrt::Popup::PopupInvoker().showInfo(
            "Info", "View creation failed", "close");
        return false;
    }

    // set cookie policy
    // even arguments pass the ewkContext, this API should be called
    // after webkit Evas_Object is created
    Ewk_Cookie_Manager *ewkCookieManager;
    ewkCookieManager =
        ewk_context_cookie_manager_get(m_ewkContext);
    ewk_cookie_manager_accept_policy_set(ewkCookieManager,
                                         EWK_COOKIE_ACCEPT_POLICY_ALWAYS);

    if (m_currentEwkView) {
        setEwkViewInvisible(m_currentEwkView);
    }

    LogDebug("push webview: " << newEwkView);
    m_ewkViewList.push_back(newEwkView);
    m_currentEwkView = newEwkView;
    return true;
}

Evas_Object* ViewLogic::addEwkView()
{
    LogDebug("enter");
    Assert(m_window);
    Assert(m_ewkContext);
    Assert(m_pageGroup);

    static Evas_Smart* evasSmart = NULL;
    static Ewk_View_Smart_Class ewkViewClass =
        EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("WRT_VIEW");

    if (!evasSmart) {
        ewk_view_smart_class_set(&ewkViewClass);

        // register webkit callbacks like window open/close
        ewkViewClass.window_create = createWindowCallback;
        ewkViewClass.window_close = closeWindowCallback;
#ifdef ORIENTATION_ENABLED
        ewkViewClass.orientation_lock = orientationLockCallback;
        ewkViewClass.orientation_unlock = orientationUnLockCallback;
#endif

        evasSmart = evas_smart_class_new(&ewkViewClass.sc);
        if (!evasSmart) {
            _D("no evas smart");
            return NULL;
        }
    }
    Evas_Object *ewkView = ewk_view_smart_add(
            evas_object_evas_get(m_window), evasSmart, m_ewkContext, m_pageGroup);
    if (!ewkView) {
        _D("no ewk view");
        return NULL;
    }

    evas_object_data_set(ewkView, EWK_SMART_CLASS_USER_DATA, this);
    return ewkView;
}

void ViewLogic::prepareEwkView(Evas_Object *wkView)
{
    LogDebug("prepareEwkView called");
    Assert(wkView);
    Ewk_Settings* settings = ewk_view_settings_get(wkView);

    // set user agent
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
    std::string customUserAgent = m_model->SettingList.Get().getUserAgent();
    if (!customUserAgent.empty()) {
        LogDebug("Setting  custom user agent as: " << customUserAgent);
        ewk_view_user_agent_set(wkView, customUserAgent.c_str());
    } else
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
    {
        LogDebug("Setting user agent as: default");
        ewk_view_user_agent_set(wkView, NULL);
        std::string defaultUA = ewk_view_user_agent_get(wkView);
        LogDebug("webkit's UA: " << defaultUA);
    }

    // set custom header : language
    using namespace ViewModule::CustomHeaderSupport;
    std::string customHeaderString = getValueByField(ACCEPT_LANGUAGE);
    if (!customHeaderString.empty()) {
        LogDebug("custom field=[" << ACCEPT_LANGUAGE << "]");
        LogDebug("custom value=[" << customHeaderString << "]");
        Eina_List* list = eina_list_append(NULL, customHeaderString.c_str());
        ewk_context_preferred_languages_set(list);
    }

    // webkit NPAPI plugins is always on in wrt
    ewk_settings_plugins_enabled_set(settings, EINA_TRUE);
    ewk_settings_javascript_enabled_set(settings, EINA_TRUE);
    ewk_settings_loads_images_automatically_set(settings, EINA_TRUE);
    // WRT should not fit web contents to device width automatically as default.
    // Fitting to device width should be handled by web content using viewport meta tag.
    ewk_settings_auto_fitting_set(settings, EINA_FALSE);
    ewk_settings_autofill_password_form_enabled_set(settings, EINA_TRUE);
    ewk_settings_form_candidate_data_enabled_set(settings, EINA_TRUE);

    evas_object_show(wkView);

    // Set transparent view only for idle clock category
    // If normal webapp accessing browser using access tag, background color should be white.
    if (m_category == 1) {
        ewk_view_bg_color_set(wkView, 0, 0, 0, 0);
    } else {
        ewk_view_bg_color_set(wkView, 0, 0, 0, 255);
    }

    std::string encoding = DEFAULT_ENCODING;
    OptionalWidgetStartFileInfo fileInfo =
        W3CFileLocalization::getStartFileInfo(m_model->TizenId);
    if (!!fileInfo) {
            std::string file_encoding = DPL::ToUTF8String((*fileInfo).encoding);

            if(EINA_TRUE == ewk_settings_is_encoding_valid(
                                                        file_encoding.c_str())){
                encoding = file_encoding;
                _D("Found custom encoding in DB: %s", encoding.c_str());
            }

    }
    _D("Setting encoding: %s", encoding.c_str());
    if (ewk_settings_default_text_encoding_name_set(settings,encoding.c_str())) {
        _D("Encoding set properly");
    } else {
        _E("Error while setting encoding");
    }

#if ENABLE(CONTENT_SECURITY_POLICY)
    if (m_model->SecurityModelVersion.Get() ==
         WrtDB::WidgetSecurityModelVersion::WIDGET_SECURITY_MODEL_V2)
    {
        // setting CSP policy rules
        DPL::OptionalString policy = m_model->CspReportOnlyPolicy.Get();
        if (!!policy) {
            LogDebug("CSP report only policy present in manifest: " << *policy);
            ewk_view_content_security_policy_set(
                wkView,
                DPL::ToUTF8String(*policy).c_str(),
                EWK_REPORT_ONLY);
        }

        policy = m_model->CspPolicy.Get();
        if (!!policy) {
            LogDebug("CSP policy present in manifest: " << *policy);
            ewk_view_content_security_policy_set(
                wkView,
                DPL::ToUTF8String(*policy).c_str(),
                EWK_ENFORCE_POLICY);
        } else {
            ewk_view_content_security_policy_set(
                wkView,
                DEFAULT_CSP_POLICY,
                EWK_ENFORCE_POLICY);
        }
    }
#endif
}

void ViewLogic::removeEwkView(Evas_Object *wkView)
{
    LogDebug("removeEwkView called");
    Assert(wkView);
    Assert(0 != m_ewkViewList.size());

    // unregister webview callbacks
    ewkClientDeinit(wkView);

    // suspend NPAPI plugin - Not implemented by Webkit2
    //    ewk_view_pause_or_resume_plugins();
    evas_object_del(wkView);
    m_ewkViewList.remove(wkView);
}

void ViewLogic::setEwkViewInvisible(Evas_Object *wkView)
{
    LogDebug("setEwkViewInvisible called");
    Assert(wkView);

    evas_object_hide(wkView);
}

void ViewLogic::setEwkViewVisible(Evas_Object *wkView)
{
    LogDebug("setEwkViewVisible called");
    Assert(wkView);

    evas_object_show(wkView);
}

void ViewLogic::resumeWebkit(Evas_Object *wkView)
{
    LogDebug("resumeWebkit : " << wkView);
    Assert(wkView);

    ewk_view_page_visibility_set(wkView, EWK_PAGE_VISIBILITY_STATE_VISIBLE, false);

    if (!m_isBackgroundSupport) {
        LogDebug("background support is disabled. resume view");
        ewk_view_resume(wkView);
        ewk_view_foreground_set(wkView, true);
    } else {
        LogDebug("background support is enabled. skip resume view");
    }

    return;
}

void ViewLogic::suspendWebkit(Evas_Object *wkView)
{
    LogDebug("suspendWebkit : " << wkView);
    Assert(wkView);

    ewk_view_page_visibility_set(wkView, EWK_PAGE_VISIBILITY_STATE_HIDDEN, false);

    if (!m_isBackgroundSupport) {
        LogDebug("background support is disabled. suspend view");
        ewk_view_suspend(wkView);
        ewk_view_foreground_set(wkView, false);
    } else {
        LogDebug("background support is enabled. skip suspend view");
    }

    return;
}

void ViewLogic::loadStartedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->loadStartedCallback) {
        This->m_cbs->loadStartedCallback(obj, eventInfo);
    }
}

void ViewLogic::loadFinishedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);

    // Fill id/password
    const char* url = ewk_view_url_get(This->m_currentEwkView);
    if (NULL == url || strlen(url) == 0) {
        LogError("url is empty");
        return;
    }

    if (This->m_cbs->loadFinishedCallback) {
        This->m_cbs->loadFinishedCallback(obj, eventInfo);
    }

    // set only encoded bundle
    double scale = elm_config_scale_get();
    ViewLogicMessageSupport::setCustomProperties(
        This->m_ewkContext,
        &scale,
        ApplicationDataSingleton::Instance().getEncodedBundle());
}

void ViewLogic::loadProgressStartedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->loadProgressStartedCallback) {
        This->m_cbs->loadProgressStartedCallback(obj, eventInfo);
    }
}

void ViewLogic::loadProgressCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);
    Assert(eventInfo);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    double* progress = static_cast<double*>(eventInfo);
    _D("progress = %f", *progress);
    if (This->m_cbs->loadProgressCallback) {
        This->m_cbs->loadProgressCallback(obj, eventInfo);
    }
}

void ViewLogic::loadProgressFinishedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->loadProgressFinishedCallback) {
        This->m_cbs->loadProgressFinishedCallback(obj, eventInfo);
    }
}

void ViewLogic::processCrashedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->processCrashedCallback) {
        This->m_cbs->processCrashedCallback(obj, eventInfo);
    }
    // This flag will be prevented exit() call in the Webkit side
    if (NULL != eventInfo) {
        *(static_cast<Eina_Bool*>(eventInfo)) = EINA_TRUE;
    }
}

Evas_Object* ViewLogic::createWindowCallback(
        Ewk_View_Smart_Data* sd,
        const Ewk_Window_Features* /*windowFeatures*/)
{
    LogDebug("createWindowCallback");
    ViewLogic* This = static_cast<ViewLogic*>(
            evas_object_data_get(sd->self, EWK_SMART_CLASS_USER_DATA));
    if (This == NULL) {
        LogError("This is null");
        return NULL;
    }

    // First, current webview should be handled by user callback
    if (This->m_cbs->unsetWebviewCallback) {
        This->m_cbs->unsetWebviewCallback(sd->self);
    }

    // create new ewkview
    This->createEwkView();
    Evas_Object* newEwkView = This->m_currentEwkView;

    // initialize new ewkview
    This->ewkClientInit(newEwkView);
    This->prepareEwkView(newEwkView);

    // Lastly, new webview should be handled by user callback
    if (This->m_cbs->setWebviewCallback) {
        This->m_cbs->setWebviewCallback(newEwkView);
    }
    return newEwkView;
}

void ViewLogic::closeWindowCallback(Ewk_View_Smart_Data* sd)
{
    LogDebug("closeWindowCallback");
    ViewLogic* This = static_cast<ViewLogic*>(
            evas_object_data_get(sd->self, EWK_SMART_CLASS_USER_DATA));
    if (This == NULL) {
        LogError("This is null");
        return;
    }
    This->m_closedEwkView = sd->self;
    ecore_idler_add(windowCloseIdlerCallback, This);
}

#ifdef ORIENTATION_ENABLED
Eina_Bool ViewLogic::orientationLockCallback(Ewk_View_Smart_Data *sd, int orientation)
{
    LogDebug("orientationLockCallback");
    ViewLogic* This = static_cast<ViewLogic*>(
            evas_object_data_get(sd->self, EWK_SMART_CLASS_USER_DATA));

    // Screen.lockOrientation
    if (This->m_orientationThresholdTimer) {
       This->m_deferredRotateAngle = orientation;

       return EINA_TRUE;
    }

    int winAngle =
        ViewModule::OrientationSupport::getWinOrientationAngle(orientation);
    if (!This->m_cbs->setOrientation.empty()) {
        This->m_cbs->setOrientation(winAngle);
    }
    ewk_context_tizen_extensible_api_set(
       This->m_ewkContext,
       EWK_EXTENSIBLE_API_ROTATION_LOCK,
       EINA_TRUE);
    This->m_orientationThresholdTimer =
        ecore_timer_add(ORIENTATION_THRESHOLD,
                        orientationThresholdTimerCallback,
                        This);

    return EINA_TRUE;
}

void ViewLogic::orientationUnLockCallback(Ewk_View_Smart_Data *sd)
{
    LogDebug("orientationUnLockCallback");
    ViewLogic* This = static_cast<ViewLogic*>(
            evas_object_data_get(sd->self, EWK_SMART_CLASS_USER_DATA));

    // Screen.unlockOrientation
    if (This->m_orientationThresholdTimer) {
        This->m_deferredRotateAngle =
            ViewModule::OrientationSupport::DEFERRED_ORIENTATION_UNLOCK;

       return;
    }

    if (This->m_model->SettingList.Get().getRotationValue() ==
        Screen_AutoRotation)
    {
        if (!This->m_cbs->setOrientation.empty()) {
            This->m_cbs->setOrientation(OrientationAngle::Window::UNLOCK);
        }
        This->m_rotateAngle = 0;
        ewk_context_tizen_extensible_api_set(
           This->m_ewkContext,
           EWK_EXTENSIBLE_API_ROTATION_LOCK,
           EINA_FALSE);
    }
}
#endif

void ViewLogic::policyNavigationDecideCallback(
    void* data,
    Evas_Object* obj,
    void* eventInfo)
{
    _D("called");

    Assert(data);
    Assert(obj);
    Assert(eventInfo);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    Ewk_Policy_Decision* policyDecision =
        static_cast<Ewk_Policy_Decision*>(eventInfo);

    // handle blocked url
    const char* url = ewk_policy_decision_url_get(policyDecision);

    // call user delegate callback
    if (This->m_cbs->policyNavigationDecideCallback) {
        if(!This->m_cbs->policyNavigationDecideCallback(obj, eventInfo)) {
            _D(" URI is blocked for DynamicBox");
            ewk_policy_decision_ignore(policyDecision);
            return;
        }
        std::string navigationUri(url);
        if (!navigationUri.compare(0, 6, SCHEME_BOX_SLASH)) {
            ewk_policy_decision_ignore(policyDecision);
            return;
        }
    }

    if (url && strlen(url) != 0) {
        if (This->m_blockedUri == url) {
            LogDebug("Blocked url = " << url);
            This->m_blockedUri = std::string();
            ewk_policy_decision_ignore(policyDecision);
            return;
        }
    }

    if (ViewModule::SchemeSupport::filterURIByScheme(policyDecision,
                                                     false,
                                                     This->m_window,
                                                     This->m_currentEwkView))
    {
        LogDebug("use");
        ewk_policy_decision_use(policyDecision);
    } else {
        // check whether this is new empty window
        const char* activeUrl = ewk_view_url_get(This->m_currentEwkView);
        if (!activeUrl || 0 == strlen(activeUrl)) {
            /*
             * The view is empty and scheme has been handled externally. When
             * user gets back from the external application he'd see blank page
             * and won't be able to navigate back. This happens when window.open
             * is used to handle schemes like sms/mms/mailto (for example in
             * WAC web standards tests: WS-15XX).
             *
             * To solve the problem, the empty view is removed from the stack
             * and the previous one is shown. This is not an elegant solution
             * but we don't have a better one.
             */
            LogDebug("Scheme has been handled externally. Removing empty view.");
            if (ewk_view_back_possible(This->m_currentEwkView)) {
                // go back to previous WKPage
                ewk_view_back(This->m_currentEwkView);
            } else {
                // stop current WKPage
                ewk_view_stop(This->m_currentEwkView);
                This->m_closedEwkView = This->m_currentEwkView;
                ecore_idler_add(windowCloseIdlerCallback, This);
            }
        }

        LogDebug("ignore");
        ewk_policy_decision_ignore(policyDecision);
    }
}

void ViewLogic::policyNewWindowDecideCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    LogDebug("policyNewWindowDecideCallback called");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    Assert(eventInfo);
    Ewk_Policy_Decision* policyDecision =
        static_cast<Ewk_Policy_Decision*>(eventInfo);

    // handle blocked url
    const char* url = ewk_policy_decision_url_get(policyDecision);
    if (url && strlen(url) != 0) {
        if (This->m_blockedUri == url) {
            LogDebug("Blocked url = " << url);
            This->m_blockedUri = std::string();
            ewk_policy_decision_ignore(policyDecision);
            return;
        }
    }

    if (ViewModule::SchemeSupport::filterURIByScheme(policyDecision,
                                                     true,
                                                     This->m_window,
                                                     This->m_currentEwkView))
    {
        ewk_policy_decision_use(policyDecision);
    } else {
        // scheme handled
        ewk_policy_decision_ignore(policyDecision);
    }
}

void ViewLogic::pageResponseDecideCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    LogDebug("pageResponseDecideCallback called");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    Assert(eventInfo);
    Ewk_Policy_Decision* policyDecision =
        static_cast<Ewk_Policy_Decision*>(eventInfo);
    Ewk_Policy_Decision_Type policyDecisionType =
        ewk_policy_decision_type_get(policyDecision);

    if (policyDecisionType == EWK_POLICY_DECISION_USE) {
        LogDebug("use");
        ewk_policy_decision_use(policyDecision);
    } else if (policyDecisionType == EWK_POLICY_DECISION_DOWNLOAD) {
        LogDebug("download");
        ewk_policy_decision_suspend(policyDecision);

        // get uri information
        const char* url = ewk_policy_decision_url_get(policyDecision);
        if (NULL == url || strlen(url) == 0) {
            LogDebug("url data is empty");
            ewk_policy_decision_use(policyDecision);
            return;
        }
        LogDebug("url = [" << url << "]");

        // get content information
        const char* content =
            ewk_policy_decision_response_mime_get(policyDecision);
        LogDebug("content type = [" << content << "]");

        // get cookie information
        const char* cookie = ewk_policy_decision_cookie_get(policyDecision);
        LogDebug("cookie = [" << cookie << "]");

        LogDebug("Content not supported, will be opened in external app");
        This->m_appsSupport->downloadRequest(
            url,
            content,
            cookie);
        ewk_policy_decision_ignore(policyDecision);
    } else if (policyDecisionType == EWK_POLICY_DECISION_IGNORE) {
        LogDebug("ignore");
        ewk_policy_decision_ignore(policyDecision);
    } else {
        LogDebug("Type isn't handled");
        ewk_policy_decision_ignore(policyDecision);
    }
}

void ViewLogic::contextmenuCustomizeCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    LogDebug("contextmenuCustomizeCallback called");
    Assert(data);
    Assert(eventInfo);
    ViewLogic* This = static_cast<ViewLogic*>(const_cast<void*>(data));
    Ewk_Context_Menu* menu = static_cast<Ewk_Context_Menu*>(eventInfo);
    if ((This->m_model->Type.Get().appType == WrtDB::APP_TYPE_TIZENWEBAPP) &&
        (This->m_model->SettingList.Get().getContextMenu()
         == ContextMenu_Disable))
    {
        LogDebug("ContextMenu Disable!!");
        for (unsigned int idx = 0; idx < ewk_context_menu_item_count(menu);) {
            Ewk_Context_Menu_Item* item = ewk_context_menu_nth_item_get(menu,
                                                                        idx);
            Assert(item);
            ewk_context_menu_item_remove(menu, item);
        }
    } else {
        LogDebug("ContextMenu Enable!!");
        unsigned int menu_num = ewk_context_menu_item_count(menu);
        unsigned int idx = 0;
        do {
            Ewk_Context_Menu_Item* item = ewk_context_menu_nth_item_get(menu,
                                                                        idx);
            if (!item) {
                idx++;
                continue;
            }
            Ewk_Context_Menu_Item_Tag tag = ewk_context_menu_item_tag_get(item);

            switch (tag) {
            case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW:
                ewk_context_menu_item_remove(menu, item);
                continue;

            case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW:
                ewk_context_menu_item_remove(menu, item);
                continue;

            case EWK_CONTEXT_MENU_ITEM_TAG_OPEN_FRAME_IN_NEW_WINDOW:
                ewk_context_menu_item_remove(menu, item);
                continue;

            case EWK_CONTEXT_MENU_ITEM_OPEN_MEDIA_IN_NEW_WINDOW:
                ewk_context_menu_item_remove(menu, item);
                continue;

            case EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB:
                ewk_context_menu_item_remove(menu, item);
                continue;

            case EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK:
                ewk_context_menu_item_remove(menu, item);
                continue;

            default:
                idx++;
                break;
            }
        } while (idx < menu_num);
    }
}

Eina_Bool ViewLogic::geolocationPermissionRequestCallback(
    Evas_Object* /*obj*/,
    Ewk_Geolocation_Permission_Request* geolocationPermissionRequest,
    void* userData)
{
    Assert(userData);
    Assert(geolocationPermissionRequest);
    ViewLogic* This = static_cast<ViewLogic*>(userData);

    DPL::OptionalBool ret =
        This->m_privilegeSupport->getPrivilegeStatus(ViewModule::PrivilegeSupport::Privilege::LOCATION);
    if (!!ret) {
        ewk_geolocation_permission_reply(geolocationPermissionRequest, (*ret) ? EINA_TRUE : EINA_FALSE);
        return EINA_TRUE;
    }

    ViewModule::GeolocationSupport::GeoLocationData geoData;
    geoData.data = userData;
    geoData.permissionRequest = geolocationPermissionRequest;
    geoData.pkgId = This->m_model->TzPkgId.Get();

    ViewModule::GeolocationSupport::geolocationPermissionRequest(
        This->m_currentEwkView,
        This->m_securityOriginSupport->getSecurityOriginDAO(),
        &geoData,
        This->m_model->TizenId);

    return EINA_TRUE;
}

void ViewLogic::notificationShowCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    Assert(eventInfo);
    Ewk_Notification* ewkNotification =
        static_cast<Ewk_Notification*>(eventInfo);
    ViewModule::WebNotificationDataPtr notiData(
        new ViewModule::WebNotificationData(ewkNotification));

    _D("notification id : %u", notiData->getEwkNotiId());
    _D("notification iconURL : %s", notiData->getIconUrl());
    _D("notification title : %s", notiData->getTitle());
    _D("notification body : %s", notiData->getBody());

    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_webNotificationSupport->show(notiData)) {
        ewk_notification_showed(This->m_ewkContext, notiData->getEwkNotiId());
    }
}

void ViewLogic::notificationCancelCallback(
    void* data,
    Evas_Object* obj,
    void* eventInfo)
{
    Assert(eventInfo);
    uint64_t ewkNotiId = *static_cast<uint64_t*>(eventInfo);

    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    Ewk_Notification* ewkNotification =
        static_cast<Ewk_Notification*>(
            This->m_webNotificationSupport->hide(ewkNotiId));
    if (ewkNotification) {
        Assert(obj);
        Eina_List* list = NULL;
        list = eina_list_append(list, ewkNotification);
        ewk_view_notification_closed(obj, list);
        eina_list_free(list);
    }
}

Eina_Bool ViewLogic::notificationPermissionRequestCallback(
    Evas_Object* obj,
    Ewk_Notification_Permission_Request* request,
    void* userData)
{
    LogDebug("called");
    Assert(userData);
    ViewLogic* This = static_cast<ViewLogic*>(userData);

    Assert(request);
    ViewModule::WebNotificationPermissionSupport::permissionRequest(
        This->m_currentEwkView,
        This->m_securityOriginSupport->getSecurityOriginDAO(),
        request);
    return EINA_TRUE;
}

// Fullscreen API callbacks
void ViewLogic::enterFullscreenCallback(
    void* data,
    Evas_Object* obj,
    void* eventInfo)
{
    _D("called");

    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewLogicMessageSupport::setViewmodes(
        This->m_ewkContext,
        "fullscreen");

    if (eventInfo) {
        This->m_isFullscreenByPlatform =
            *static_cast<Eina_Bool*>(eventInfo) == EINA_TRUE;
    }
    if (This->m_cbs->enterFullscreenCallback) {
        This->m_cbs->enterFullscreenCallback(obj, eventInfo);
    }
}

void ViewLogic::exitFullscreenCallback(
    void* data,
    Evas_Object* obj,
    void* eventInfo)
{
    _D("called");

    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewLogicMessageSupport::setViewmodes(
        This->m_ewkContext,
        Message::ToInjectedBundle::SET_VIEWMODES_MSGBODY_EXIT);

    This->m_isFullscreenByPlatform = false;
    if (This->m_cbs->exitFullscreenCallback) {
        This->m_cbs->exitFullscreenCallback(obj, eventInfo);
    }
}

void ViewLogic::enabledVideoHwOverlayCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->enableVideoHwOverlayCallback) {
        This->m_cbs->enableVideoHwOverlayCallback(obj, eventInfo);
    }
}

void ViewLogic::disabledVideoHwOverlayCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->disableVideoHwOverlayCallback) {
        This->m_cbs->disableVideoHwOverlayCallback(obj, eventInfo);
    }
}

void ViewLogic::imeChangedCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    LogDebug("enter");
    Assert(data);
    Assert(eventInfo);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    Eina_Rectangle *rect = static_cast<Eina_Rectangle *>(eventInfo);
    This->m_imeWidth = rect->w;
    This->m_imeHeight = rect->h;
}

void ViewLogic::imeOpenedCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* /*eventInfo*/)
{
    LogDebug("enter");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);

    using namespace WrtPlugins::W3C;
    SoftKeyboardChangeArgs args;
    args.state = IME_STATE_ON;
    args.width = This->m_imeWidth;
    args.height = This->m_imeHeight;
    This->fireJavascriptEvent(
        static_cast<int>(SoftKeyboardChangeCustomEvent),
        &args);
}

void ViewLogic::imeClosedCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* /*eventInfo*/)
{
    LogDebug("enter");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);

    using namespace WrtPlugins::W3C;
    SoftKeyboardChangeArgs args;
    args.state = IME_STATE_OFF;
    This->fireJavascriptEvent(
        static_cast<int>(SoftKeyboardChangeCustomEvent),
        &args);
}

Eina_Bool ViewLogic::usermediaPermissionRequestCallback(
    Evas_Object* /*obj*/,
    Ewk_User_Media_Permission_Request* request,
    void* userData)
{
    LogDebug("called");
    Assert(userData);
    ViewLogic* This = static_cast<ViewLogic*>(userData);

    DPL::OptionalBool ret =
        This->m_privilegeSupport->getPrivilegeStatus(ViewModule::PrivilegeSupport::Privilege::MEDIACAPTURE);
    if (!!ret) {
        ewk_user_media_permission_reply(request, (*ret) ? EINA_TRUE : EINA_FALSE);
        return EINA_TRUE;
    }

    ViewModule::UsermediaSupport::usermediaPermissionRequest(
        This->m_currentEwkView,
        This->m_securityOriginSupport->getSecurityOriginDAO(),
        request,
        This->m_model->TizenId);
    return EINA_TRUE;
}

// helper method
CustomHandlerDB::CustomHandlerPtr getCustomHandlerFromData(void* data)
{
    Assert(data);
    Ewk_Custom_Handlers_Data* handler =
        static_cast<Ewk_Custom_Handlers_Data*>(data);
    CustomHandlerDB::CustomHandlerPtr customHandler(
        new CustomHandlerDB::CustomHandler());
    const char* base_url = ewk_custom_handlers_data_base_url_get(handler);
    if (base_url) {
        LogDebug("base url: " << base_url);
        customHandler->base_url = DPL::FromASCIIString(string(base_url));
    }
    const char* url = ewk_custom_handlers_data_url_get(handler);
    if (url) {
        LogDebug("url: " << url);
        customHandler->url = DPL::FromASCIIString(string(url));
    }
    const char* target = ewk_custom_handlers_data_target_get(handler);
    if (target) {
        LogDebug("target: " << target);
        customHandler->target = DPL::FromASCIIString(string(target));
    }
    const char* title = ewk_custom_handlers_data_title_get(handler);
    if (title) {
        LogDebug("title: " << title);
        customHandler->title = DPL::FromASCIIString(string(title));
    }
    return customHandler;
}

void ViewLogic::attachToCustomHandlersDao()
{
    if (!m_attachedToCustomHandlerDao) {
        CustomHandlerDB::Interface::attachDatabaseRW();
    }
}

void ViewLogic::detachFromCustomHandlersDao()
{
    if (m_attachedToCustomHandlerDao) {
        CustomHandlerDB::Interface::detachDatabase();
    }
}

const int protocolWhiteListLenth = 15;
char const * const protocolWhiteList[protocolWhiteListLenth] = {
    "irc",
    "geo",
    "mailto",
    "magnet",
    "mms",
    "news",
    "nntp",
    "sip",
    "sms",
    "smsto",
    "ssh",
    "tel",
    "urn",
    "webcal",
    "xmpp"
};

const int contentBlackListLenth = 14;
char const * const contentBlackList[contentBlackListLenth] = {
    "application/x-www-form-urlencoded",
    "application/xhtml+xml",
    "application/xml",
    "image/gif",
    "image/jpeg",
    "image/png",
    "image/svg+xml",
    "multipart/x-mixed-replace",
    "text/cache-manifest",
    "text/css",
    "text/html",
    "text/ping",
    "text/plain",
    "text/xml"
};

/**
 * Saves user's response from popup to custom handler. Saves Yes/No and remember
 * state.
 * @param response
 * @param customHandler
 */
void saveUserResponse(Wrt::Popup::PopupResponse response,
                      CustomHandlerDB::CustomHandlerPtr customHandler)
{
    switch (response) {
    case Wrt::Popup::YES_DO_REMEMBER:
        LogDebug("User allowed, remember");
        customHandler->user_decision =
            static_cast<CustomHandlerDB::HandlerState>
            (CustomHandlerDB::Agreed | CustomHandlerDB::DecisionSaved);
        break;
    case Wrt::Popup::YES_DONT_REMEMBER:
        LogDebug("User allowed, don't remember");
        customHandler->user_decision = CustomHandlerDB::Agreed;
        break;
    case Wrt::Popup::NO_DO_REMEMBER:
        LogDebug("User didn't allow, remember");
        customHandler->user_decision =
            static_cast<CustomHandlerDB::HandlerState>
            (CustomHandlerDB::Declined | CustomHandlerDB::DecisionSaved);
        break;
    case Wrt::Popup::NO_DONT_REMEMBER:
        LogDebug("User didn't allow, don't remember");
        customHandler->user_decision = CustomHandlerDB::Declined;
        break;
    }
}

//TODO registration, checking if registered and unregistration can be done in
//common functions for both types of handlers. Only white and black lists
//have to be separated
//TODO attach database only one at the start (not in every callback?)
void ViewLogic::protocolHandlerRegistrationCallback(void* data,
                                                    Evas_Object* /*obj*/,
                                                    void* eventInfo)
{
    Assert(data);
    LogDebug("enter");
    CustomHandlerDB::CustomHandlerPtr customHandler =
        getCustomHandlerFromData(eventInfo);

    std::string scheme = DPL::ToUTF8String(customHandler->target);
    if (scheme.empty()) {
        LogError("No scheme provided");
        //TODO what about securityError?
        return;
    }
    bool matched = false;
    //scheme on whiteList
    for (int i = 0; i < protocolWhiteListLenth; ++i) {
        if (0 == strcmp(protocolWhiteList[i], scheme.c_str())) {
            LogDebug("Match found, protocol can be handled");
            matched = true;
        }
    }
    if (!matched) {
        //starts with web+ and have at least 5 chars (lowercase ASCII)
        if (strncmp("web+", scheme.c_str(), 4) || scheme.length() < 5) {
            LogWarning("Scheme neither on whitelist nor starts with \"web+\"");
            //throw SecurityException
            return;
        }
        int l = 4;
        char c = scheme[l];
        while (c != '\0') {
            if (c < 'a' || c > 'z') {
                LogWarning("Wrong char inside scheme. "
                           << "Only lowercase ASCII letters accepted");
                //throw SecurityException
                return;
            }
            c = scheme[++l];
        }
    }

    ViewLogic* This = static_cast<ViewLogic*>(data);
    LogDebug("Creating handlers dao");
    This->attachToCustomHandlersDao();
    CustomHandlerDB::CustomHandlerDAO handlersDao(This->m_model->TizenId);
    CustomHandlerDB::CustomHandlerPtr handler =
        handlersDao.getProtocolHandler(customHandler->target,
                                       customHandler->url,
                                       customHandler->base_url);
    if (handler && (handler->user_decision & CustomHandlerDB::DecisionSaved)) {
        LogDebug("Protocol already registered - nothing to do");
    } else {
        LogDebug("Protocol handler not found");
        Wrt::Popup::PopupResponse response =
            GlobalSettings::PopupsTestModeEnabled() ? Wrt::Popup::
                YES_DO_REMEMBER :
            Wrt::Popup::PopupInvoker().askYesNoCheckbox(
                PROTOCOL_HANDLER_ASK_TITLE,
                PROTOCOL_HANDLER_ASK_MSG,
                PROTOCOL_HANDLER_ASK_REMEMBER);
        saveUserResponse(response, customHandler);
        if (customHandler->user_decision == CustomHandlerDB::Declined) {
            return;
        }
        if (customHandler->user_decision & CustomHandlerDB::Agreed) {
            //TODO remove old default handler somehow from appsvc
            LogDebug("Registering appservice entry");
            int ret = appsvc_set_defapp(APPSVC_OPERATION_VIEW,
                                        NULL,
                                        DPL::ToUTF8String(
                                            customHandler->target).c_str(),
                                        DPL::ToUTF8String(This->m_model->
                                                              TizenId).c_str());
            if (APPSVC_RET_OK != ret) {
                LogWarning("Appsvc entry failed: " << ret);
                //no database change
                return;
            }
        }
        handlersDao.registerProtocolHandler(*(customHandler.get()));

        LogDebug("Protocal saved");
    }

    This->detachFromCustomHandlersDao();
}

void ViewLogic::protocolHandlerIsRegisteredCallback(void* data,
                                                    Evas_Object* /*obj*/,
                                                    void* eventInfo)
{
    LogDebug("enter");
    CustomHandlerDB::CustomHandlerPtr customHandler = getCustomHandlerFromData(
            eventInfo);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    LogDebug("Creating handlers dao");
    This->attachToCustomHandlersDao();
    CustomHandlerDB::CustomHandlerDAO handlersDao(This->m_model->TizenId);
    CustomHandlerDB::CustomHandlerPtr handler =
        handlersDao.getProtocolHandler(customHandler->target,
                                       customHandler->url,
                                       customHandler->base_url);
    if (handler) {
        if (handler->user_decision & CustomHandlerDB::Agreed) {
            ewk_custom_handlers_data_result_set(
                static_cast<Ewk_Custom_Handlers_Data*>(eventInfo),
                EWK_CUSTOM_HANDLERS_REGISTERED);
        } else {
            ewk_custom_handlers_data_result_set(
                static_cast<Ewk_Custom_Handlers_Data*>(eventInfo),
                EWK_CUSTOM_HANDLERS_DECLINED);
        }
    } else {
        ewk_custom_handlers_data_result_set(
            static_cast<Ewk_Custom_Handlers_Data*>(eventInfo),
            EWK_CUSTOM_HANDLERS_NEW);
    }
    This->detachFromCustomHandlersDao();
}

void ViewLogic::protocolHandlerUnregistrationCallback(void* data,
                                                      Evas_Object* /*obj*/,
                                                      void* eventInfo)
{
    LogDebug("enter");
    CustomHandlerDB::CustomHandlerPtr customHandler =
        getCustomHandlerFromData(eventInfo);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    LogDebug("Creating handlers dao");
    This->attachToCustomHandlersDao();
    CustomHandlerDB::CustomHandlerDAO handlersDao(This->m_model->TizenId);
    CustomHandlerDB::CustomHandlerPtr handlerCheck =
        handlersDao.getProtocolHandler(customHandler->target,
                                       customHandler->url,
                                       customHandler->base_url);
    This->detachFromCustomHandlersDao();
    if (handlerCheck) {
        if (handlerCheck->user_decision & CustomHandlerDB::Agreed) {
            int ret = appsvc_unset_defapp(
                    DPL::ToUTF8String(This->m_model->TizenId).c_str());
            if (APPSVC_RET_OK != ret) {
                LogWarning("Failed to unregister appsvc entry");
                return;
            }
        }
        //if appsvc ok change custom_handlers_db
        handlersDao.unregisterProtocolHandler(customHandler->target,
                                              customHandler->url,
                                              customHandler->base_url);
    } else {
        LogDebug("Nothing to unregister");
    }
}

void ViewLogic::contentHandlerRegistrationCallback(void* data,
                                                   Evas_Object* /*obj*/,
                                                   void* eventInfo)
{
    Assert(data);
    LogDebug("enter");
    CustomHandlerDB::CustomHandlerPtr customHandler =
        getCustomHandlerFromData(eventInfo);

    std::string mimeType = DPL::ToUTF8String(customHandler->target);
    if (mimeType.empty()) {
        LogError("No mimeType provided.");
        return;
    }
    for (int i = 0; i < contentBlackListLenth; ++i) {
        if (0 == strcmp(contentBlackList[i], mimeType.c_str())) {
            LogWarning("mimeType blacklisted");
            //throw SecurityException
            return;
        }
    }

    ViewLogic* This = static_cast<ViewLogic*>(data);
    LogDebug("Creating handlers dao");
    This->attachToCustomHandlersDao();
    CustomHandlerDB::CustomHandlerDAO handlersDao(This->m_model->TizenId);
    CustomHandlerDB::CustomHandlerPtr handler =
        handlersDao.getContentHandler(customHandler->target,
                                      customHandler->url,
                                      customHandler->base_url);
    if (handler && (handler->user_decision & CustomHandlerDB::DecisionSaved)) {
        LogDebug("Protocol already registered - nothing to do");
    } else {
        LogDebug("Protocol handler not found");
        Wrt::Popup::PopupResponse response =
            GlobalSettings::PopupsTestModeEnabled() ? Wrt::Popup::
                YES_DO_REMEMBER :
            Wrt::Popup::PopupInvoker().askYesNoCheckbox(
                CONTENT_HANDLER_ASK_TITLE,
                CONTENT_HANDLER_ASK_MSG,
                CONTENT_HANDLER_AKS_REMEMBER);
        saveUserResponse(response, customHandler);
        if (customHandler->user_decision == CustomHandlerDB::Declined) {
            return;
        }
        if (customHandler->user_decision & CustomHandlerDB::Agreed) {
            //TODO remove old default handler somehow from appsvc
            LogDebug("Registering appservice entry");
            int ret = appsvc_set_defapp(APPSVC_OPERATION_VIEW,
                                        DPL::ToUTF8String(
                                            customHandler->target).c_str(),
                                        NULL,
                                        DPL::ToUTF8String(This->m_model->
                                                              TizenId).c_str());
            if (APPSVC_RET_OK != ret) {
                LogWarning("Appsvc entry failed: " << ret);
                return;
            }
        }
        handlersDao.registerContentHandler(*(customHandler.get()));
        LogDebug("Content saved");
    }
    This->detachFromCustomHandlersDao();
}

void ViewLogic::contentHandlerIsRegisteredCallback(void* data,
                                                   Evas_Object* /*obj*/,
                                                   void* eventInfo)
{
    LogDebug("enter");
    CustomHandlerDB::CustomHandlerPtr customHandler =
        getCustomHandlerFromData(eventInfo);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    LogDebug("Creating handlers dao");

    This->attachToCustomHandlersDao();
    CustomHandlerDB::CustomHandlerDAO handlersDao(This->m_model->TizenId);
    CustomHandlerDB::CustomHandlerPtr handler =
        handlersDao.getContentHandler(customHandler->target,
                                      customHandler->url,
                                      customHandler->base_url);
    if (handler) {
        if (handler->user_decision & CustomHandlerDB::Agreed) {
            ewk_custom_handlers_data_result_set(
                static_cast<Ewk_Custom_Handlers_Data*>(eventInfo),
                EWK_CUSTOM_HANDLERS_REGISTERED);
        } else {
            ewk_custom_handlers_data_result_set(
                static_cast<Ewk_Custom_Handlers_Data*>(eventInfo),
                EWK_CUSTOM_HANDLERS_DECLINED);
        }
    } else {
        ewk_custom_handlers_data_result_set(
            static_cast<Ewk_Custom_Handlers_Data*>(eventInfo),
            EWK_CUSTOM_HANDLERS_NEW);
    }
    This->detachFromCustomHandlersDao();
}

void ViewLogic::contentHandlerUnregistrationCallback(void* data,
                                                     Evas_Object* /*obj*/,
                                                     void* eventInfo)
{
    LogDebug("enter");
    CustomHandlerDB::CustomHandlerPtr customHandler =
        getCustomHandlerFromData(eventInfo);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    LogDebug("Creating handlers dao");
    This->attachToCustomHandlersDao();
    CustomHandlerDB::CustomHandlerDAO handlersDao(This->m_model->TizenId);
    CustomHandlerDB::CustomHandlerPtr handlerCheck =
        handlersDao.getContentHandler(customHandler->target,
                                      customHandler->url,
                                      customHandler->base_url);
    This->detachFromCustomHandlersDao();
    if (handlerCheck) {
        if (handlerCheck->user_decision & CustomHandlerDB::Agreed) {
            int ret = appsvc_unset_defapp(
                    DPL::ToUTF8String(This->m_model->TizenId).c_str());
            if (APPSVC_RET_OK != ret) {
                LogWarning("Failed to unregister mime handler from appsvc");
                return;
            }
        }
        handlersDao.unregisterContentHandler(customHandler->target,
                                             customHandler->url,
                                             customHandler->base_url);
    } else {
        LogDebug("Nothing to unregister");
    }
}

void ViewLogic::didRunJavaScriptCallback(
    Evas_Object* /*obj*/,
    const char* result,
    void* /*userData*/)
{
    LogDebug("didRunJavaScriptCallback called");
    LogDebug("result = " << result);
}

void ViewLogic::eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);
    Assert(obj);

    ViewLogic* This = static_cast<ViewLogic*>(data);

    Ea_Callback_Type keyType =
        static_cast<Ea_Callback_Type>(reinterpret_cast<int>(eventInfo));

    LogDebug("Key = [" << keyType << "]");

    std::string keyName;
    if (keyType == EA_CALLBACK_BACK) {
        Assert(obj);
        // Call fullscreen exit API
        // In case of fullscreen is entered by platform(default video tag),
        // automatically exit fullscreen when backkey is selected
        if (This->m_isFullscreenByPlatform) {
            ewk_view_fullscreen_exit(obj);
            return;
        }

        // Call text selection clear API
        // In case of current state is selection mode,
        // application doesn't need to handle back key
        if (EINA_TRUE == ewk_view_text_selection_clear(obj)) {
            return;
        }
        keyName = KeyName::BACK;
    } else if (keyType == EA_CALLBACK_MORE) {
        keyName = KeyName::MENU;
    } else {
        return;
    }

    if (This->m_model->SettingList.Get().getHWkeyEvent() == HWkeyEvent_Enable)
    {
        DispatchEventSupport::dispatchHwKeyEvent(obj, keyName);
    }
    if (This->m_cbs->keyCallback) {
        This->m_cbs->keyCallback(obj, eventInfo);
    }

    return;
}

Eina_Bool ViewLogic::rotaryCallback(void* /*data*/, Evas_Object *obj, Eext_Rotary_Event_Info *info)
{
    Assert(obj);

    std::stringstream script;

    script << "var __event = document.createEvent(\"CustomEvent\");\n"
           << "var __detail = {};\n"
           << "__event.initCustomEvent(\"rotarydetent\", true, true, __detail);\n"
           << "__event.detail.direction = \"" << (info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE ? "CW" : "CCW") << "\";\n"
           << "document.dispatchEvent(__event);\n"
           << "\n"
           << "for (var i=0; i < window.frames.length; i++)\n"
           << "{ window.frames[i].document.dispatchEvent(__event); }";

    // just for debugging
    LogDebug("script :\n" << script.str());

    if (ewk_view_script_execute(obj, script.str().c_str(), NULL, NULL) != EINA_TRUE)
    {
        LogWarning("ewk_view_script_execute returned FALSE!");
    }

    return true;
}

Eina_Bool ViewLogic::windowCloseIdlerCallback(void* data)
{
    LogDebug("closeIdlerCallback");
    ViewLogic* This = static_cast<ViewLogic*>(data);
    This->windowClose();
    return ECORE_CALLBACK_CANCEL;
}

Eina_Bool ViewLogic::exceededDatabaseQuotaCallback(Evas_Object* obj,
                                                   Ewk_Security_Origin* origin,
                                                   const char* ,
                                                   unsigned long long ,
                                                   void* data)
{
    LogDebug("exceededDatabaseQuotaCallback called");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewModule::WebStorageSupport::createPermissionRequest(
        This->m_currentEwkView,
        This->m_securityOriginSupport->getSecurityOriginDAO(),
        obj,
        origin,
        ewk_view_exceeded_database_quota_reply);
    return EINA_TRUE;
}

Eina_Bool ViewLogic::exceededIndexedDatabaseQuotaCallback(Evas_Object* obj,
                                                          Ewk_Security_Origin* origin,
                                                          long long ,
                                                          void* data)
{
    LogDebug("exceededIndexedDatabaseQuotaCallback called");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewModule::WebStorageSupport::createPermissionRequest(
        This->m_currentEwkView,
        This->m_securityOriginSupport->getSecurityOriginDAO(),
        obj,
        origin,
        ewk_view_exceeded_indexed_database_quota_reply);
    return EINA_TRUE;
}

Eina_Bool ViewLogic::exceededLocalFileSystemQuotaCallback(Evas_Object* obj,
                                                          Ewk_Security_Origin* origin,
                                                          long long ,
                                                          void* data)
{
    LogDebug("exceededLocalFileSystemQuotaCallback called");
    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewModule::WebStorageSupport::createPermissionRequest(
        This->m_currentEwkView,
        This->m_securityOriginSupport->getSecurityOriginDAO(),
        obj,
        origin,
        ewk_view_exceeded_local_file_system_quota_reply);
    return EINA_TRUE;
}

void ViewLogic::certificateConfirmRequestCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    LogDebug("certificateConfirmRequestCallback called");

    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    Assert(eventInfo);
    ViewModule::CertificateConfirmSupport::certificatePermissionRequest(
        This->m_currentEwkView,
        This->m_certificateSupport->getCertificateDAO(),
        eventInfo);
}

void ViewLogic::authenticationRequestCallback(
    void* data,
    Evas_Object* /*obj*/,
    void* eventInfo)
{
    LogDebug("authenticationRequestCallback called");
    Assert(data);
    Assert(eventInfo);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    const char* url = ewk_view_url_get(This->m_currentEwkView);
    if (!url || strlen(url) == 0) {
        Ewk_Auth_Request* authRequest = static_cast<Ewk_Auth_Request*>(eventInfo);
        ewk_auth_request_cancel(authRequest);
        return;
    }
    ViewModule::AuthenticationRequestSupport::authenticationRequest(
        This->m_currentEwkView,
        url,
        eventInfo);
}

void ViewLogic::viewFrameRenderedCallback(
    void* data,
    Evas_Object* obj,
    void* eventInfo)
{
    _D("enter");

    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);

    if (This->m_cbs->frameRenderedCallback) {
        This->m_cbs->frameRenderedCallback(obj, eventInfo);
    }
}

#ifdef ORIENTATION_ENABLED
void ViewLogic::mediacontrolRotateHorizontal(void* data,
                                             Evas_Object* obj,
                                             void* /*eventInfo*/)
{
    LogDebug("mediacontrolRotateHorizontal called");
    Assert(data);
    Assert(obj);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewModule::OrientationSupport::setEwkOrientation(
        obj,
        OrientationAngle::W3C::Landscape::PRIMARY);
    if (This->m_cbs->orientationLockCallback) {
        This->m_cbs->orientationLockCallback(OrientationAngle::Window::Landscape::PRIMARY);
    }
}

void ViewLogic::mediacontrolRotateVertical(void* data,
                                           Evas_Object* obj,
                                           void* /*eventInfo*/)
{
    LogDebug("mediacontrolRotateVertical called");
    Assert(data);
    Assert(obj);
    ViewLogic* This = static_cast<ViewLogic*>(data);
    ViewModule::OrientationSupport::setEwkOrientation(
        obj,
        OrientationAngle::W3C::Portrait::PRIMARY);
    if (This->m_cbs->orientationLockCallback) {
        This->m_cbs->orientationLockCallback(OrientationAngle::Window::Portrait::PRIMARY);
    }
}

void ViewLogic::mediacontrolRotateExit(void* data,
                                       Evas_Object* obj,
                                       void* /*eventInfo*/)
{
    LogDebug("mediacontrolRotateExit called");
    Assert(data);
    Assert(obj);
    ViewLogic* This = static_cast<ViewLogic*>(data);

    int w3cAngle = 0;
    int winAngle = 0;
    if (This->m_rotateAngle == 0) {
        // application hasn't call orientation lock
        WidgetSettingScreenLock screenLock =
            This->m_model->SettingList.Get().getRotationValue();
        if (screenLock == Screen_Portrait) {
            w3cAngle = OrientationAngle::W3C::Portrait::PRIMARY;
            winAngle = OrientationAngle::Window::Portrait::PRIMARY;
        } else if (screenLock == Screen_Landscape) {
            w3cAngle = OrientationAngle::W3C::Landscape::PRIMARY;
            winAngle = OrientationAngle::Window::Landscape::PRIMARY;
        } else if (screenLock == Screen_AutoRotation) {
            if (This->m_cbs->orientationLockCallback) {
                This->m_cbs->orientationLockCallback(OrientationAngle::Window::UNLOCK);
            }
            return;
        }
    } else {
        // Restore previous orientation
        w3cAngle =
            ViewModule::OrientationSupport::getW3COrientationAngle(
                This->m_rotateAngle);
        winAngle =
            ViewModule::OrientationSupport::getWinOrientationAngle(
                This->m_rotateAngle);
    }

    ViewModule::OrientationSupport::setEwkOrientation(obj, w3cAngle);
    if (This->m_cbs->orientationLockCallback) {
        This->m_cbs->orientationLockCallback(winAngle);
    }

}

Eina_Bool ViewLogic::orientationThresholdTimerCallback(void* data)
{
    LogDebug("orientationThresholdTimerCallback");
    ViewLogic* This = static_cast<ViewLogic*>(data);

    if (This->m_deferredRotateAngle ==
        ViewModule::OrientationSupport::DEFERRED_ORIENTATION_EMPTY)
    {
        // There is no defered orientation API call
        This->m_orientationThresholdTimer = NULL;
        return ECORE_CALLBACK_CANCEL;
    }

    if (This->m_deferredRotateAngle != This->m_rotateAngle) {
        This->m_rotateAngle = This->m_deferredRotateAngle;
        int w3cAngle = 0;
        int winAngle = 0;
        if (This->m_rotateAngle == 0) {
            WidgetSettingScreenLock screenLock =
                This->m_model->SettingList.Get().getRotationValue();
            if (screenLock == Screen_Portrait) {
                w3cAngle = OrientationAngle::W3C::Portrait::PRIMARY;
                winAngle = OrientationAngle::Window::Portrait::PRIMARY;
            } else if (screenLock == Screen_Landscape) {
                w3cAngle = OrientationAngle::W3C::Landscape::PRIMARY;
                winAngle = OrientationAngle::Window::Landscape::PRIMARY;
            } else if (screenLock == Screen_AutoRotation) {
                if (This->m_cbs->orientationLockCallback) {
                    This->m_cbs->orientationLockCallback(OrientationAngle::Window::UNLOCK);
                }
                This->m_orientationThresholdTimer = NULL;
                return ECORE_CALLBACK_CANCEL;
            }
        } else {
            // Restore previous orientation
            w3cAngle =
                ViewModule::OrientationSupport::getW3COrientationAngle(
                    This->m_rotateAngle);
            winAngle =
                ViewModule::OrientationSupport::getWinOrientationAngle(
                    This->m_rotateAngle);
        }

        ViewModule::OrientationSupport::setEwkOrientation(
            This->m_currentEwkView,
            w3cAngle);
        if (This->m_cbs->orientationLockCallback) {
            This->m_cbs->orientationLockCallback(winAngle);
        }
        This->m_deferredRotateAngle =
            ViewModule::OrientationSupport::DEFERRED_ORIENTATION_EMPTY;
        return ECORE_CALLBACK_RENEW;
    }

    This->m_orientationThresholdTimer = NULL;
    return ECORE_CALLBACK_CANCEL;
}
#endif

void ViewLogic::popupReplyWaitStart(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    This->m_isPopupReplyWait = true;
    if (This->m_cbs->popupReplyWaitStartCallback) {
        This->m_cbs->popupReplyWaitStartCallback(obj, eventInfo);
    }
}

void ViewLogic::popupReplyWaitFinish(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    This->m_isPopupReplyWait = false;
    if (This->m_cbs->popupReplyWaitFinishCallback) {
        This->m_cbs->popupReplyWaitFinishCallback(obj, eventInfo);
    }
}

void ViewLogic::consoleMessageCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->consoleMessageCallback) {
        This->m_cbs->consoleMessageCallback(obj, eventInfo);
    }
}

#ifdef ORIENTATION_ENABLED
void ViewLogic::rotatePreparedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    Assert(data);

    ViewLogic* This = static_cast<ViewLogic*>(data);
    if (This->m_cbs->rotatePreparedCallback) {
        This->m_cbs->rotatePreparedCallback(obj, eventInfo);
    }
}
#endif

void ViewLogic::windowClose()
{
    LogDebug("windowClose");
    AssertMsg(m_closedEwkView, "no closed webview");

    if (1 >= m_ewkViewList.size()) {
        if (m_cbs->processExitCallback) {
            m_cbs->processExitCallback(m_closedEwkView, NULL);
        }
    } else {
        // call user callbacks
        if (m_cbs->unsetWebviewCallback) {
            m_cbs->unsetWebviewCallback(m_currentEwkView);
        }
        removeEwkView(m_closedEwkView);

        // get latest ewkView
        m_currentEwkView = m_ewkViewList.back();

        setEwkViewVisible(m_currentEwkView);

        // show ewkView
        if (m_cbs->setWebviewCallback) {
            m_cbs->setWebviewCallback(m_currentEwkView);
        }
    }
}

void ViewLogic::systemSettingsChangedCallback(system_settings_key_e key,
                                              void* data)
{
    LogDebug("systemSettingsChanged");
    LogDebug("System setting Key is [" << key << "]");

    Assert(data);
    ViewLogic* This = static_cast<ViewLogic*>(data);

    if (SYSTEM_SETTINGS_KEY_FONT_TYPE == key) {
        if (!This->m_currentEwkView) {
            LogError("ewkView isn't initialized");
            return;
        }
        ewk_view_use_settings_font(This->m_currentEwkView);
    } else {
        LogError("Unregister system callback is called");
    }
}

void ViewLogic::vibrateCallback(void* data, Evas_Object* /*obj*/, void* eventInfo)
{
    ViewLogic* This = static_cast<ViewLogic*>(data);
    unsigned* vibrationTime = static_cast<unsigned*>(eventInfo);
    This->activateVibration(true, static_cast<unsigned long>(*vibrationTime));
}

void ViewLogic::cancelVibrationCallback(void* data, Evas_Object* /*obj*/, void* /*eventInfo*/)
{
    ViewLogic* This = static_cast<ViewLogic*>(data);
    This->activateVibration(false, 0);
}


