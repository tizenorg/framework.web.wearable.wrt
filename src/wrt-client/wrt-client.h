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
#ifndef WRT_CLIENT_H
#define WRT_CLIENT_H

#include <memory>
#include <string>

#include <bundle.h>
#include <dpl/application.h>
#include <dpl/platform.h>
#include <dpl/event/controller.h>
#include <dpl/generic_event.h>
#include <dpl/task.h>
#include <dpl/type_list.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <Elementary.h>
#include <Evas.h>
#include <Ecore.h>
#include <i_runnable_widget_object.h>
#include <widget_data_types.h>

#include "widget_state.h"
#include "window_data.h"

DECLARE_GENERIC_EVENT_0(NextStepEvent)

namespace ClientModule {
class SubmodeSupport;
class OrientationSupport;
}
class SplashScreenSupport;
namespace WrtDB {
class WidgetDAOReadOnly;
}
class WidgetSettingList;

class WrtClient :
    public DPL::Application,
    private DPL::Event::Controller<DPL::TypeListDecl<NextStepEvent>::Type>,
    public DPL::TaskDecl<WrtClient>
{
  public:
    WrtClient(int argc,
              char **argv);
    virtual ~WrtClient();

    class ReturnStatus
    {
      public:
        enum Type
        {
            Failed = -1,
            Succeeded = 0
        };
    };

    ReturnStatus::Type   getReturnStatus() const;
    virtual void Quit();
    static std::string getTizenIdFromArgument(int argc, char **argv);

  protected:
    virtual void OnStop();
    virtual void OnCreate();
    virtual void OnResume();
    virtual void OnPause();
    virtual void OnReset(bundle *b);
    virtual void OnTerminate();

    void setStep();

  private:
    enum LaunchStatus {
        LAUNCH_STATUS_FAIL = -2,
        LAUNCH_STATUS_DELAYED = -1,
        LAUNCH_STATUS_FINISHED,
    };

    void showHelpAndQuit();
    void setDebugMode(bundle* b);
    void initializeWindowModes();
    AppCategory checkAppCategory();

    // Process Pool
    void sendLaunchStatusToPoolServer(LaunchStatus status);
    void setTimeoutForLaunchDelayedStatus();
    void setTimeoutForLaunchFinishedStatus();
    void setTimeoutFirstFrameRenderedDelay();
    void unsetTimeout(Ecore_Timer** timer);
    void fireLanguageChangePopup(float delay);

    // Events
    virtual void OnEventReceived(const NextStepEvent& event);

    // static Callback
    static Eina_Bool naviframeBackButtonCallback(void* data,
                                                 Elm_Object_Item* it);
    static int appcoreLowMemoryCallback(void* data);
    static int languageChangedCallback(void *data, void *tmp);
#ifdef ORIENTATION_ENABLED
    static void autoRotationCallback(void* data, Evas_Object* obj, void* event);
    static void initialOrientationCheckCallback(void* data, Evas_Object* obj, void* event);
#endif
    static void focusedCallback(void* data,
                                Evas_Object* obj,
                                void* eventInfo);
    static void unfocusedCallback(void* data,
                                  Evas_Object* obj,
                                  void* eventInfo);
    static Eina_Bool processPoolFdCallback(void* data, Ecore_Fd_Handler *handler);
    static Eina_Bool processPoolLaunchDelayedTimerCallback(void* data);
    static Eina_Bool processPoolLaunchFinishedTimerCallback(void* data);
    static Eina_Bool firstFrameRenderedDelayTimerCallback(void* data);
    static int checkAppCategoryCallback(const char* name, void* result);

    //view-mode
    void setInitialViewMode();

#ifdef ORIENTATION_ENABLED
    //orientation
    void setWindowOrientation(int angle);
    void initWindowOrientation();
    void deinitWindowOrientation();
#endif

    // hwkey
    void keyCallback(Evas_Object* obj, void* eventInfo);

    void consoleMessageCallback(Evas_Object* obj, void* eventInfo);
#ifdef ORIENTATION_ENABLED
    void rotatePreparedCallback(Evas_Object* obj, void* eventInfo);
#endif

    // launching steps
    void initStep();
    void launchStep();
    void shutdownStep();
    void showStep();
    void switchToShutdownStep();
    void loadStartedCallback(Evas_Object* obj, void* eventInfo);
    void loadFinishedCallback(Evas_Object* obj, void* eventInfo);
    void loadProgressStartedCallback(Evas_Object* obj, void* eventInfo);
    void loadProgressCallback(Evas_Object* obj, void* eventInfo);
    void loadProgressFinishedCallback(Evas_Object* obj, void* eventInfo);
    void processExitCallback(Evas_Object* obj, void* eventInfo);
    void processCrashedCallback(Evas_Object* obj, void* eventInfo);
    void enterFullscreenCallback(Evas_Object* obj, void* eventInfo);
    void exitFullscreenCallback(Evas_Object* obj, void* eventInfo);
    void enableVideoHwOverlayCallback(Evas_Object* obj, void* eventInfo);
    void disableVideoHwOverlayCallback(Evas_Object* obj, void* eventInfo);
    void setLayout(Evas_Object* newBuffer);
    void unsetLayout(Evas_Object* currentBuffer);
    void popupReplyWaitStartCallback(Evas_Object* obj, void* eventInfo);
    void popupReplyWaitFinishCallback(Evas_Object* obj, void* eventInfo);
    void frameRenderedCallback(Evas_Object* obj, void* eventInfo);
    void blockedUrlPolicyCallback(const std::string& blockedUrl) ;

    // Private data
    std::string m_tizenId;

    bool m_launched;
    bool m_initializing;
    bool m_initialized;
    bool m_debugMode;
    ReturnStatus::Type m_returnStatus;
    WRT::RunnableWidgetObjectPtr m_widget;
    WrtDB::WidgetDAOReadOnlyPtr m_dao;
    WidgetSettingListPtr m_settingList;
    WidgetState m_widgetState;
    WindowDataPtr m_windowData;
    std::unique_ptr<SplashScreenSupport> m_splashScreen;
    std::string m_initialViewMode;
    std::string m_currentViewMode;
    bool m_isWebkitFullscreen;
    Ecore_Timer* m_processPoolLaunchDelayedTimer;
    Ecore_Timer* m_processPoolLaunchFinishedTimer;
    Ecore_Timer* m_firstFrameRenderedDelayTimer;

#if USE(WEBKIT_MANUAL_ROTATION)
    bool m_isWebkitHandleManualRotationDone;
#endif
    std::unique_ptr<ClientModule::SubmodeSupport> m_submodeSupport;
#ifdef ORIENTATION_ENABLED
    std::unique_ptr<ClientModule::OrientationSupport> m_orientationSupport;
#endif
};

#endif // WRT_CLIENT_H
