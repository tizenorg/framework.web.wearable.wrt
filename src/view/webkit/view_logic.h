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
 * @file    view_logic.h
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @brief   Declaration file for view logic for Webkit2
 */

#ifndef VIEW_LOGIC_H_
#define VIEW_LOGIC_H_

#include <i_view_module.h>
#include <memory>
#include <map>
#include <string>
#include <dpl/log/log.h>
#include <dpl/assert.h>
#include <dpl/platform.h>

#include <widget_model.h>
#include <i_runnable_widget_object.h>
#include <common/view_logic_apps_support.h>
#include <common/view_logic_vibration_support.h>
#include <system_settings.h>

#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace ViewModule {
class SecurityOriginSupport;
class CertificateSupport;
class WebNotificationSupport;
}

class ViewLogic : public ViewModule::IViewModule
{
  public:
    ViewLogic();
    virtual ~ViewLogic();

    // IViewModule Impl
    bool createView(Ewk_Context* context,
                       Evas_Object* window);
    void destroyWebView();
    void prepareView(WidgetModel* m, const std::string &startUrl, int category);
    void showWidget();
    void hideWidget();
    void suspendWidget();
    void resumeWidget();
    void resetWidgetFromSuspend();
    void resetWidgetFromResume();
    void backward();
    void reloadStartPage();
    Evas_Object* getCurrentWebview();
    void fireJavascriptEvent(int event, void* data);
    void setUserCallbacks(const WRT::UserDelegatesPtr& cbs);
    void checkSyncMessageFromBundle(
            const char* name,
            const char* body,
            char** returnData);
    void checkAsyncMessageFromBundle(
            const char* name,
            const char* body);
    void downloadData(const char* url);
    void activateVibration(bool on, uint64_t time);


  private:
    void initializeSupport();
    void initializePluginLoading();
    void initializeXwindowHandle();
    void resetWidgetCommon();

    // EwkView operations
    Evas_Object* addEwkView();
    void ewkClientInit(Evas_Object *wkView);
    void ewkClientDeinit(Evas_Object *wkView);
    bool createEwkView();
    void prepareEwkView(Evas_Object *wkView);
    void removeEwkView(Evas_Object *wkView);
    void setEwkViewVisible(Evas_Object *wkView);
    void setEwkViewInvisible(Evas_Object *wkView);
    void resumeWebkit(Evas_Object *wkView);
    void suspendWebkit(Evas_Object *wkView);

    // WKPageLoaderClient
    static void loadStartedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void loadFinishedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void loadProgressStartedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void loadProgressCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void loadProgressFinishedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void processCrashedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // EWK Callback
    static Evas_Object* createWindowCallback(
        Ewk_View_Smart_Data *sd,
        const Ewk_Window_Features *window_features);
    static void closeWindowCallback(Ewk_View_Smart_Data *sd);

    // EWK PolicyDecide Callback
    static void policyNavigationDecideCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void policyNewWindowDecideCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void pageResponseDecideCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // EWK ContextMenu Callback
    static void contextmenuCustomizeCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // EWK Geolocation Callback
    static void geolocationPermissionRequestCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // EWK Notification Callback
    static void notificationShowCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void notificationCancelCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void notificationPermissionRequestCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

#ifdef ORIENTATION_ENABLED
    // EWK Orientation Callback
    static Eina_Bool orientationLockCallback(
        Ewk_View_Smart_Data *smartData,
        int orientations);

    static void orientationUnLockCallback(
        Ewk_View_Smart_Data *smartData);
#endif
    // EWK Fullscreen API callbacks
    static void enterFullscreenCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void exitFullscreenCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    static void enabledVideoHwOverlayCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    static void disabledVideoHwOverlayCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // EWK IME Change/Show/Hide Callback
    static void imeChangedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void imeOpenedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);
    static void imeClosedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // EWK Usermedia Callback
    static void usermediaPermissionRequestCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    // custom content/scheme handlers
    static void protocolHandlerRegistrationCallback(void* data,
                                                    Evas_Object* obj,
                                                    void* eventInfo);
    static void protocolHandlerIsRegisteredCallback(void* data,
                                                    Evas_Object* obj,
                                                    void* eventInfo);
    static void protocolHandlerUnregistrationCallback(void* data,
                                                      Evas_Object* obj,
                                                      void* eventInfo);

    static void contentHandlerRegistrationCallback(void* data,
                                                   Evas_Object* obj,
                                                   void* eventInfo);
    static void contentHandlerIsRegisteredCallback(void* data,
                                                   Evas_Object* obj,
                                                   void* eventInfo);
    static void contentHandlerUnregistrationCallback(void* data,
                                                     Evas_Object* obj,
                                                     void* eventInfo);
    static Eina_Bool exceededDatabaseQuotaCallback(Evas_Object* obj,
                                                   Ewk_Security_Origin* origin,
                                                   const char* databaseName,
                                                   unsigned long long expectedQuota,
                                                   void* data);
    static Eina_Bool exceededIndexedDatabaseQuotaCallback(Evas_Object* obj,
                                                          Ewk_Security_Origin* origin,
                                                          long long expectedQuota,
                                                          void* data);
    static Eina_Bool exceededLocalFileSystemQuotaCallback(Evas_Object* obj,
                                                          Ewk_Security_Origin* origin,
                                                          long long expectedQuota,
                                                          void* data);
    static void certificateConfirmRequestCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    static void authenticationRequestCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

    static void viewFrameRenderedCallback(
        void* data,
        Evas_Object* obj,
        void* eventInfo);

#ifdef ORIENTATION_ENABLED
    static void mediacontrolRotateHorizontal(void* data,
                                             Evas_Object* obj,
                                             void* eventInfo);
    static void mediacontrolRotateVertical(void* data,
                                           Evas_Object* obj,
                                           void* eventInfo);
    static void mediacontrolRotateExit(void* data,
                                       Evas_Object* obj,
                                       void* eventInfo);
#endif
    static void popupReplyWaitStart(void* data,
                                    Evas_Object* obj,
                                    void* eventInfo);
    static void popupReplyWaitFinish(void* data,
                                     Evas_Object* obj,
                                     void* eventInfo);
    static void consoleMessageCallback(void* data,
                                       Evas_Object* obj,
                                       void* eventInfo);

#ifdef ORIENTATION_ENABLED
    static void rotatePreparedCallback(void* data, Evas_Object* obj, void* eventInfo);
#endif

    void attachToCustomHandlersDao();
    void detachFromCustomHandlersDao();

    // JS execute callback
    static void didRunJavaScriptCallback(
        Evas_Object* obj,
        const char* result,
        void* userData);

    // event callback
    static void eaKeyCallback(void* data,
                              Evas_Object* obj,
                              void* eventInfo);

    // idler callback
    static Eina_Bool windowCloseIdlerCallback(void *data);

#ifdef ORIENTATION_ENABLED
    // timer callback
    static Eina_Bool orientationThresholdTimerCallback(void* data);
#endif

    // window
    void windowClose(void);

    // system settings
    static void systemSettingsChangedCallback(system_settings_key_e key, void* data);

    // vibration callback
    static void vibrateCallback(void* data, Evas_Object* obj, void* eventInfo);
    static void cancelVibrationCallback(void* data, Evas_Object* obj, void* eventinfo);

    Ewk_Context* m_ewkContext;
    bool m_attachedToCustomHandlerDao;
    std::list<Evas_Object*> m_ewkViewList;
    Evas_Object* m_currentEwkView;
    Evas_Object* m_closedEwkView;
    Evas_Object* m_window;
    WidgetModel* m_model;
    std::string m_blockedUri;
    std::string m_theme;
    std::string m_startUrl;
    WRT::UserDelegatesPtr m_cbs;
    size_t m_imeWidth;
    size_t m_imeHeight;
    bool m_isBackgroundSupport;
#ifdef ORIENTATION_ENABLED
    int m_rotateAngle;
    int m_deferredRotateAngle;
    Ecore_Timer* m_orientationThresholdTimer;
#endif
    // TODO: change name to m_isWebkitBlocked
    bool m_isPopupReplyWait;
    Ewk_Page_Group* m_pageGroup;
    bool m_isFullscreenByPlatform;
    int m_category;

    std::unique_ptr<ViewModule::AppsSupport> m_appsSupport;
    std::unique_ptr<ViewModule::VibrationSupport> m_vibrationSupport;
    std::unique_ptr<ViewModule::SecurityOriginSupport> m_securityOriginSupport;
    std::unique_ptr<ViewModule::CertificateSupport> m_certificateSupport;
    std::unique_ptr<ViewModule::WebNotificationSupport> m_webNotificationSupport;

    static std::map<const std::string, const Evas_Smart_Cb> m_ewkCallbacksMap;
};

#endif //VIEW_LOGIC_H_
