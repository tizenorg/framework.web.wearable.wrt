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

#include "wrt-client.h"

#include <cstdlib>
#include <cstdio>
#include <string>
#include <sys/time.h>
#include <sys/resource.h>

#include <aul.h>
#include <app_control.h>
#include <appcore-efl.h>
#include <appcore-common.h>
#include <dlog/dlog.h>
#include <dpl/availability.h>
#include <dpl/bind.h>
#include <dpl/exception.h>
#include <dpl/localization/w3c_file_localization.h>
#include <dpl/localization/LanguageTagsProvider.h>
#include <dpl/log/log.h>
#include <dpl/log/secure_log.h>
#include <dpl/optional_typedefs.h>
#include <dpl/platform.h>
#include <efl_assist.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <popup-runner/PopupInvoker.h>
#include <vconf.h>
#include <pkgmgr-info.h>
#include <device.h>

#include <application_data.h>
#include <core_module.h>
#include <localization_setting.h>
#include <process_pool.h>
#include <process_pool_launchpad_util.h>
#include <user_delegates.h>
#include <widget_deserialize_model.h>
#include <widget_string.h>
#include "client_command_line_parser.h"
#include "client_ide_support.h"
#ifdef ORIENTATION_ENABLED
#include "client_orientation_support.h"
#endif
#include "client_security_support.h"
#include "client_submode_support.h"
#include "splash_screen_support.h"

#ifdef JOURNAL_LOG_SUPPORT
 #include <journal/appcore.h>
#endif

//W3C PACKAGING enviroment variable name
#define W3C_DEBUG_ENV_VARIABLE "DEBUG_LOAD_FINISH"

// window signal callback
const char *EDJE_SHOW_PROGRESS_SIGNAL = "show,progress,signal";
const char *EDJE_HIDE_PROGRESS_SIGNAL = "hide,progress,signal";
const char *EDJE_1X1_RATIO_SIGNAL = "1x1,ratio,signal";
const std::string VIEWMODE_TYPE_FULLSCREEN = "fullscreen";
const std::string VIEWMODE_TYPE_MAXIMIZED = "maximized";
const std::string VIEWMODE_TYPE_WINDOWED = "windowed";
char const* const ELM_SWALLOW_CONTENT = "elm.swallow.content";
const char* const BUNDLE_PATH = "/usr/lib/libwrt-injected-bundle.so";
const char* const MESSAGE_NAME_INITIALIZE = "ToInjectedBundle::INIT";
const unsigned int UID_ROOT = 0;

// process pool
const char* const DUMMY_PROCESS_PATH = "/usr/bin/wrt_launchpad_daemon_candidate";
static Ewk_Context* s_preparedEwkContext = NULL;
static WindowData*  s_preparedWindowData = NULL;
static int    app_argc = 0;
static char** app_argv = NULL;
static bool s_processPoolDisabled = false;
static int s_clientFd = -1;
static const float s_processPoolLaunchDelayedWaitTime = 5.0f;
static const float s_processPoolLaunchFinishedWaitTime = 0.35f;
static const float s_firstFrameRenderedDelayWaitTime = 2.0f;
// env
const char* const HOME = "HOME";
const char* const APP_HOME_PATH = "/opt/home/app";
const char* const ROOT_HOME_PATH = "/opt/home/root";
const char* const WRT_CONSOLE_LOG_ENABLE = "WRT_CONSOLE_LOG_ENABLE";

const char* CATEGORY_IDLE_CLOCK = "com.samsung.wmanager.WATCH_CLOCK";
const char* CATEGORY_WEARABLE_CLOCK = "http://tizen.org/category/wearable_clock";

WrtClient::WrtClient(int argc, char **argv) :
    Application(argc, argv, "wrt-client", false),
    DPL::TaskDecl<WrtClient>(this),
    m_launched(false),
    m_initializing(false),
    m_initialized(false),
    m_debugMode(false),
    m_returnStatus(ReturnStatus::Succeeded),
    m_widgetState(WidgetState::WidgetState_Stopped),
    m_initialViewMode(VIEWMODE_TYPE_MAXIMIZED),
    m_currentViewMode(VIEWMODE_TYPE_MAXIMIZED),
    m_isWebkitFullscreen(false),
    m_processPoolLaunchDelayedTimer(NULL),
    m_processPoolLaunchFinishedTimer(NULL),
    m_firstFrameRenderedDelayTimer(NULL),
#if USE(WEBKIT_MANUAL_ROTATION)
    m_isWebkitHandleManualRotationDone(false),
#endif
    m_submodeSupport(new ClientModule::SubmodeSupport())
{
    Touch();
    LogDebug("App Created");
}

WrtClient::~WrtClient()
{
    LogDebug("App Finished");
}

WrtClient::ReturnStatus::Type WrtClient::getReturnStatus() const
{
    return m_returnStatus;
}

void WrtClient::OnStop()
{
    LogDebug("Stopping Dummy Client");
}

void WrtClient::OnCreate()
{
    LogDebug("On Create");
    ADD_PROFILING_POINT("OnCreate callback", "point");

    setTimeoutForLaunchDelayedStatus();
    setTimeoutFirstFrameRenderedDelay();

    ewk_init();
}

void WrtClient::OnResume()
{
    if (m_widgetState != WidgetState_Suspended) {
        LogWarning("Widget is not suspended, resuming was skipped");
        return;
    }

    m_widget->Resume();
    m_widgetState = WidgetState_Running;
}

void WrtClient::OnPause()
{
    if (m_widgetState != WidgetState_Running) {
        LogWarning("Widget is not running to be suspended");
        return;
    }
    if (m_submodeSupport->isNeedTerminateOnSuspend()) {
        LogDebug("Current mode cannot support suspend");
        elm_exit();
        return;
    }
    m_widget->Suspend();
    m_widgetState = WidgetState_Suspended;

    //FIX.ME This code should be rearranged.
    device_set_brightness_from_settings(0);
}

// appcore does not send low memory event to remove sluggish issue.
// Add callback to vconf to get low memory event.
static void vconf_lowmem_handler(keynode_t* key, void* data);

void WrtClient::OnReset(bundle *b)
{
    LogDebug("OnReset");

    // bundle argument is freed after OnReset() is returned
    // So bundle duplication is needed
    ApplicationDataSingleton::Instance().setBundle(bundle_dup(b));
    ApplicationDataSingleton::Instance().setEncodedBundle(b);

    if (true == m_initializing) {
        LogDebug("can not handle reset event");
        return;
    }
    if (true == m_launched) {
        if (m_widgetState == WidgetState_Stopped) {
            LogError("Widget is not running to be reset");
            return;
        }
        m_widget->Reset();
#ifdef ORIENTATION_ENABLED
        m_orientationSupport->resetOrientation();
#endif
        m_widgetState = WidgetState_Running;
    } else {
        m_tizenId =
            ClientModule::CommandLineParser::getTizenId(m_argc, m_argv);
        if (m_tizenId.empty()) {
            showHelpAndQuit();
        } else {
            setDebugMode(b);
            setStep();
        }
    }

// appcore does not send low memory event to remove sluggish issue.
// Add callback to vconf to get low memory event.
#if 0
    // low memory callback set
    appcore_set_event_callback(
            APPCORE_EVENT_LOW_MEMORY,
            WrtClient::appcoreLowMemoryCallback,
            this);
#else
    // register low memory changed callback
    vconf_notify_key_changed(VCONFKEY_SYSMAN_LOW_MEMORY, vconf_lowmem_handler, this);
#endif
}

void WrtClient::OnTerminate()
{
    LogDebug("Wrt Shutdown now");
    shutdownStep();
    //FIX.ME This code should be rearranged.
    device_set_brightness_from_settings(0);
}

void WrtClient::showHelpAndQuit()
{
    printf("Usage: wrt-client [OPTION]... [WIDGET: ID]...\n"
           "launch widgets.\n"
           "Mandatory arguments to long options are mandatory for short "
           "options too.\n"
           "  -h,    --help                                 show this help\n"
           "  -l,    --launch                               "
           "launch widget with given tizen ID\n"
           "  -t,    --tizen                                "
           "launch widget with given tizen ID\n"
           "\n");

    Quit();
}

void WrtClient::setStep()
{
    LogDebug("setStep");

    AddStep(&WrtClient::initStep);
    AddStep(&WrtClient::launchStep);
    AddStep(&WrtClient::showStep);
    AddStep(&WrtClient::shutdownStep);

    m_initializing = true;

    DPL::Event::ControllerEventHandler<NextStepEvent>::PostEvent(NextStepEvent());
}

void WrtClient::setDebugMode(bundle* b)
{
    m_debugMode = ClientModule::IDESupport::getDebugMode(b);
    LogDebug("debug mode : " << m_debugMode);
}

void WrtClient::OnEventReceived(const NextStepEvent& /*event*/)
{
    LogDebug("Executing next step");
    NextStep();
}

void WrtClient::initStep()
{
    LogDebug("");
    if (WRT::CoreModuleSingleton::Instance().Init()) {
        m_initialized = true;
    } else {
        m_returnStatus = ReturnStatus::Failed;
        switchToShutdownStep();
        return;
    }

    DPL::Event::ControllerEventHandler<NextStepEvent>::PostEvent(NextStepEvent());
}

void WrtClient::loadStartedCallback(Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

#if USE(WEBKIT_SHOW_PROGRESS_BAR_EARLIER)
    if (m_settingList->getProgressBarPresence() == ProgressBar_Enable ||
        m_initialViewMode == VIEWMODE_TYPE_WINDOWED)
    {
        m_windowData->signalEmit(Layer::MAIN_LAYOUT,
                                 EDJE_SHOW_PROGRESS_SIGNAL,
                                 "");
        m_windowData->updateProgress(0);
    }
#endif
}

void WrtClient::loadFinishedCallback(Evas_Object* obj, void* eventInfo)
{
    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    ADD_PROFILING_POINT("loadFinishedCallback", "start");

    // Splash screen
    if (m_splashScreen && m_splashScreen->isShowing())
    {
        m_splashScreen->stopSplashScreenBuffered();
    }

    LogDebug("Post result of launch");

    //w3c packaging test debug (message on 4>)
    const char * makeScreen = getenv(W3C_DEBUG_ENV_VARIABLE);
    if (makeScreen != NULL && strcmp(makeScreen, "1") == 0) {
        FILE* doutput = fdopen(4, "w");
        fprintf(doutput, "didFinishLoadForFrameCallback: ready\n");
        fclose(doutput);
    }

    LogDebug("Launch succesfull");

    m_launched = true;
    m_initializing = false;
    setlinebuf(stdout);
    ADD_PROFILING_POINT("loadFinishedCallback", "stop");
    printf("launched\n");
    fflush(stdout);

    if (m_debugMode) {
        unsigned int portNum =
            ewk_view_inspector_server_start(obj, 0);
        LogDebug("Port for inspector : " << portNum);
        bool ret = ClientModule::IDESupport::sendReply(
                       ApplicationDataSingleton::Instance().getBundle(),
                       portNum);
        if (!ret) {
            LogWarning("Fail to send reply");
        }
    }

    ApplicationDataSingleton::Instance().freeBundle();
}

void WrtClient::loadProgressStartedCallback(Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

#if !USE(WEBKIT_SHOW_PROGRESS_BAR_EARLIER)
    if (m_settingList->getProgressBarPresence() == ProgressBar_Enable ||
        m_initialViewMode == VIEWMODE_TYPE_WINDOWED)
    {
        m_windowData->signalEmit(Layer::MAIN_LAYOUT,
                                 EDJE_SHOW_PROGRESS_SIGNAL,
                                 "");
        m_windowData->updateProgress(0);
    }
#endif
}

void WrtClient::loadProgressCallback(Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(obj);

    if (m_settingList->getProgressBarPresence() == ProgressBar_Enable ||
        m_initialViewMode == VIEWMODE_TYPE_WINDOWED)
    {
        Assert(eventInfo);
        double* progress = static_cast<double*>(eventInfo);
        m_windowData->updateProgress(*progress);
    }
}

void WrtClient::loadProgressFinishedCallback(Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    if (m_settingList->getProgressBarPresence() == ProgressBar_Enable ||
        m_initialViewMode == VIEWMODE_TYPE_WINDOWED)
    {
        m_windowData->signalEmit(Layer::MAIN_LAYOUT,
                                 EDJE_HIDE_PROGRESS_SIGNAL,
                                 "");
    }
}

void WrtClient::processExitCallback(Evas_Object* obj, void* eventInfo)
{
    _D("process exit");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    elm_exit();
}

void WrtClient::processCrashedCallback(Evas_Object* obj, void* eventInfo)
{
    LogError("webProcess crashed");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    switchToShutdownStep();
}

void WrtClient::enterFullscreenCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    // enter fullscreen
    m_windowData->toggleFullscreen(true);
    m_currentViewMode = VIEWMODE_TYPE_FULLSCREEN;
    m_isWebkitFullscreen = true;
}

void WrtClient::exitFullscreenCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    // exit fullscreen
    m_windowData->toggleFullscreen(false);
    m_currentViewMode = m_initialViewMode;
    m_isWebkitFullscreen = false;
}

void WrtClient::enableVideoHwOverlayCallback(Evas_Object* obj, void* eventInfo)
{
    LogDebug("called");

    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    ewk_view_bg_color_set(obj, 0, 0, 0, 0);
    m_windowData->toggleTransparent(true);
}

void WrtClient::disableVideoHwOverlayCallback(Evas_Object* obj, void* eventInfo)
{
    LogDebug("called");

    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    m_windowData->toggleTransparent(false);
    ewk_view_bg_color_set(obj, 0, 0, 0, 255);
}

void WrtClient::popupReplyWaitStartCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

#ifdef ORIENTATION_ENABLED
#if USE(WEBKIT_MANUAL_ROTATION)
    m_orientationSupport->setManualRotation(false);
    ewk_context_tizen_extensible_api_set(s_preparedEwkContext,
                                         EWK_EXTENSIBLE_API_PRERENDERING_FOR_ROTATION,
                                         EINA_FALSE);
#endif
#endif
}

void WrtClient::popupReplyWaitFinishCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

#ifdef ORIENTATION_ENABLED
#if USE(WEBKIT_MANUAL_ROTATION)
    m_orientationSupport->setManualRotation(true);
    ewk_context_tizen_extensible_api_set(s_preparedEwkContext,
                                         EWK_EXTENSIBLE_API_PRERENDERING_FOR_ROTATION,
                                         EINA_TRUE);
#endif
#endif
}

void WrtClient::frameRenderedCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

#ifdef ORIENTATION_ENABLED
#if USE(WEBKIT_MANUAL_ROTATION)
    if (m_isWebkitHandleManualRotationDone == false) {
        _D("start manual roatation");
        m_orientationSupport->setManualRotation(true);
        m_isWebkitHandleManualRotationDone = true;
        ewk_context_tizen_extensible_api_set(s_preparedEwkContext,
                                             EWK_EXTENSIBLE_API_PRERENDERING_FOR_ROTATION,
                                             EINA_TRUE);
    }
#endif
#endif

    static bool isFirstCalled = true;
    if (!isFirstCalled) {
        return;
    }
    isFirstCalled = false;

#ifdef JOURNAL_LOG_SUPPORT
    journal_appcore_app_fully_loaded(const_cast<char*>(m_tizenId.c_str()));
#endif

    LogDebug("first frame has been made");
    // send status to process pool server
    // this should be done only one time!
    if (m_processPoolLaunchDelayedTimer) {
        // timer for checking delay launch is not expired yet
        // so wrt-client stops the timer
        unsetTimeout(&m_processPoolLaunchDelayedTimer);
        // start timer for waiting several seconds after load is finished
        // now this is 1 second
        setTimeoutForLaunchFinishedStatus();
    }

    // check if window is already showed, or not
    if (m_firstFrameRenderedDelayTimer) {
        // this case means that frame rendered callback is not delayed
        unsetTimeout(&m_firstFrameRenderedDelayTimer);
        LogDebug("window show!");
        // show window
        evas_object_show(m_windowData->getEvasObject(Layer::WINDOW));
    }
}

void WrtClient::blockedUrlPolicyCallback (const std::string& blockedUrl)
{
    // block this page and open it in browser
    _D("Request was blocked : %s", blockedUrl.c_str());

    app_control_h app_control = NULL;
    app_control_create(&app_control);
    app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
    app_control_set_uri(app_control, blockedUrl.c_str());

    if(APP_CONTROL_ERROR_NONE != app_control_send_launch_request(app_control, NULL, NULL)) {
        _E("Failed to run app_control");
    }

    app_control_destroy(app_control);
    return;
}

void WrtClient::launchStep()
{
    ADD_PROFILING_POINT("launchStep", "start");
    LogDebug("Launching widget ...");

    ADD_PROFILING_POINT("getRunnableWidgetObject", "start");
    m_widget = WRT::CoreModuleSingleton::Instance()
            .getRunnableWidgetObject(m_tizenId);
    ADD_PROFILING_POINT("getRunnableWidgetObject", "stop");

    if (!m_widget) {
        LogError("RunnableWidgetObject is NULL, stop launchStep");
        switchToShutdownStep();
        return;
    }

    if (m_widgetState == WidgetState_Running) {
        LogWarning("Widget already running, stop launchStep");
        switchToShutdownStep();
        return;
    }

    if (m_widgetState == WidgetState_Authorizing) {
        LogWarning("Widget already authorizing, stop launchStep");
        switchToShutdownStep();
        return;
    }

    m_dao.reset(new WrtDB::WidgetDAOReadOnly(DPL::FromASCIIString(m_tizenId)));
    WrtDB::WidgetSettings widgetSettings;
    m_dao->getWidgetSettings(widgetSettings);
    m_settingList.reset(new WidgetSettingList(widgetSettings));
    m_submodeSupport->initialize(DPL::FromASCIIString(m_tizenId));

    DPL::OptionalString defloc = m_dao->getDefaultlocale();
    if (!!defloc) {
        LanguageTagsProviderSingleton::Instance().addWidgetDefaultLocales(
            *defloc);
    }

    setInitialViewMode();

    LocalizationSetting::SetLanguageChangedCallback(
            languageChangedCallback, this);

    AppCategory type = checkAppCategory();

    ADD_PROFILING_POINT("CreateWindow", "start");

    LogDebug("Category: " << type);
    if( type == APP_CATEGORY_IDLE_CLOCK ){
        std::unique_ptr<char, void(*)(void*)> idleClockStr(vconf_get_str(VCONFKEY_WMS_CLOCKS_SET_IDLE), std::free);
        std::string pkgid = DPL::ToUTF8String(m_dao->getTizenPkgId());
        if( idleClockStr.get() == NULL || strcmp(idleClockStr.get(), pkgid.c_str()) != 0 ){
            LogError("Does not allowed to launch idle clock app");
            switchToShutdownStep();
            return;
        }
        m_windowData.reset(new WindowData(type, static_cast<unsigned long>(getpid()), true));
        WindowData::MiniControlCallbackPtr callback(new WindowData::MiniControlCallback);
        callback->resumeCallback = DPL::Bind(&WrtClient::OnResume, this);
        callback->suspendCallback = DPL::Bind(&WrtClient::OnPause, this);
        m_windowData->setMiniControlCallback(callback);
        if( s_preparedWindowData != NULL ){
            delete s_preparedWindowData;
            s_preparedWindowData = NULL;
        }
    }else{
        if (s_preparedWindowData == NULL) {
            s_preparedWindowData = new WindowData(type, static_cast<unsigned long>(getpid()), true);
        }
        m_windowData.reset(s_preparedWindowData);
        s_preparedWindowData = NULL;
    }
    ADD_PROFILING_POINT("CreateWindow", "stop");
    if (!m_windowData->initScreenReaderSupport(
            m_settingList->getAccessibility() == Accessibility_Enable))
    {
        LogWarning("Fail to set screen reader support set");
    }

#ifdef ORIENTATION_ENABLED
    // rotate window to initial value
    initWindowOrientation();
#endif

    WRT::UserDelegatesPtr cbs(new WRT::UserDelegates);

    ADD_PROFILING_POINT("Create splash screen", "start");
    DPL::OptionalString splashImgSrc = m_dao->getSplashImgSrc();
    if (!!splashImgSrc)
    {
        m_splashScreen.reset(
            new SplashScreenSupport(
                m_windowData->getEvasObject(Layer::WINDOW),
                (DPL::ToUTF8String(*splashImgSrc)).c_str(),
                m_currentViewMode != VIEWMODE_TYPE_FULLSCREEN,
                m_settingList->getRotationValue() == Screen_Landscape));
        m_splashScreen->startSplashScreen();
        evas_object_show(this->m_windowData->getEvasObject(Layer::WINDOW));
    }
    ADD_PROFILING_POINT("Create splash screen", "stop");

    DPL::OptionalString startUrl = W3CFileLocalization::getStartFile(m_dao);
#if USE(WEB_PROVIDER_EXCEPTION_IN_EWK_CONTEXT)
    ewk_context_tizen_extensible_api_set(s_preparedEwkContext, EWK_EXTENSIBLE_API_MEDIA_VOLUME_CONTROL, EINA_TRUE);
#endif
    if (!m_widget->PrepareView(
            DPL::ToUTF8String(*startUrl),
            m_windowData->getEvasObject(Layer::WINDOW),
            s_preparedEwkContext, type))
    {
        switchToShutdownStep();
        return;
    }

#ifdef ORIENTATION_ENABLED
    // send rotate information to ewk
    m_orientationSupport->setEwkInitialOrientation(
            m_settingList->getRotationValue());
#endif

    //you can't show window with splash screen before PrepareView
    //ewk_view_add_with_context() in viewLogic breaks window
    m_windowData->init();
    m_windowData->postInit();

    // only for gear3 product
    DPL::OptionalString value = m_dao->getPropertyValue(L"view-compat");
    if (!!value && value == L"square") {
        m_windowData->signalEmit(Layer::MAIN_LAYOUT, EDJE_1X1_RATIO_SIGNAL, "");
    }

    // sub-mode support
    if (m_submodeSupport->isInlineMode()) {
        if (m_submodeSupport->transientWindow(
                elm_win_xwindow_get(
                    m_windowData->getEvasObject(Layer::WINDOW))))
        {
            LogDebug("Success to set submode");
        } else {
            LogWarning("Fail to set submode");
        }
    }
    m_windowData->smartCallbackAdd(Layer::FOCUS,
                                   "focused",
                                   focusedCallback,
                                   this);
    m_windowData->smartCallbackAdd(Layer::FOCUS,
                                   "unfocused",
                                   unfocusedCallback,
                                   this);

    WrtDB::WidgetLocalizedInfo localizedInfo =
        W3CFileLocalization::getLocalizedInfo(m_dao);
    std::string name = "";
    if (!!localizedInfo.name) {
        name = DPL::ToUTF8String(*(localizedInfo.name));
    }
    elm_win_title_set(m_windowData->getEvasObject(Layer::WINDOW),
                      name.c_str());

    initializeWindowModes();

    m_widgetState = WidgetState_Authorizing;
    if (!m_widget->CheckBeforeLaunch()) {
        LogError("CheckBeforeLaunch failed, stop launchStep");
        switchToShutdownStep();
        return;
    }
    LogDebug("Widget launch accepted. Entering running state");
    m_widgetState = WidgetState_Running;

    cbs->loadProgressStartedCallback = DPL::Bind(&WrtClient::loadProgressStartedCallback, this);
    cbs->loadProgressCallback = DPL::Bind(&WrtClient::loadProgressCallback, this);
    cbs->loadProgressFinishedCallback = DPL::Bind(&WrtClient::loadProgressFinishedCallback, this);
    cbs->loadStartedCallback = DPL::Bind(&WrtClient::loadStartedCallback, this);
    cbs->loadFinishedCallback = DPL::Bind(&WrtClient::loadFinishedCallback, this);
    cbs->setWebviewCallback = DPL::Bind(&WrtClient::setLayout, this);
    cbs->unsetWebviewCallback = DPL::Bind(&WrtClient::unsetLayout, this);
    cbs->processExitCallback = DPL::Bind(&WrtClient::processExitCallback, this);
    cbs->processCrashedCallback = DPL::Bind(&WrtClient::processCrashedCallback, this);
    cbs->enterFullscreenCallback = DPL::Bind(&WrtClient::enterFullscreenCallback, this);
    cbs->exitFullscreenCallback = DPL::Bind(&WrtClient::exitFullscreenCallback, this);
#ifdef ORIENTATION_ENABLED
    cbs->orientationLockCallback = DPL::Bind(&WrtClient::setWindowOrientation, this);
#endif
    cbs->keyCallback = DPL::Bind(&WrtClient::keyCallback, this);
    cbs->consoleMessageCallback = DPL::Bind(&WrtClient::consoleMessageCallback, this);
#ifdef ORIENTATION_ENABLED
    cbs->rotatePreparedCallback = DPL::Bind(&WrtClient::rotatePreparedCallback, this);
#endif
    cbs->enableVideoHwOverlayCallback = DPL::Bind(&WrtClient::enableVideoHwOverlayCallback, this);
    cbs->disableVideoHwOverlayCallback = DPL::Bind(&WrtClient::disableVideoHwOverlayCallback, this);
    cbs->popupReplyWaitStartCallback = DPL::Bind(&WrtClient::popupReplyWaitStartCallback, this);
    cbs->popupReplyWaitFinishCallback = DPL::Bind(&WrtClient::popupReplyWaitFinishCallback, this);
    cbs->frameRenderedCallback = DPL::Bind(&WrtClient::frameRenderedCallback, this);
    cbs->blockedUrlPolicyCallback = DPL::Bind(&WrtClient::blockedUrlPolicyCallback, this);

    m_widget->SetUserDelegates(cbs);

#ifdef ORIENTATION_ENABLED
    if (m_orientationSupport->isOrientationPrepared(m_settingList->getRotationValue())) {
        DPL::Event::ControllerEventHandler<NextStepEvent>::PostEvent(NextStepEvent());
    } else {
        m_orientationSupport->registerRotationCallback(initialOrientationCheckCallback, this);
    }
#else
    DPL::Event::ControllerEventHandler<NextStepEvent>::PostEvent(NextStepEvent());
#endif
    ADD_PROFILING_POINT("launchStep", "stop");
}

void WrtClient::showStep()
{
    _D("Show widget ...");
    m_widget->Show();
}

void WrtClient::switchToShutdownStep()
{
    SwitchToStep(&WrtClient::shutdownStep);
    DPL::Event::ControllerEventHandler<NextStepEvent>::PostEvent(NextStepEvent());
}

void WrtClient::initializeWindowModes()
{
    Assert(m_windowData);
    bool backbutton =
        (m_settingList->getBackButtonPresence() == BackButton_Enable ||
        m_initialViewMode == VIEWMODE_TYPE_WINDOWED);
    m_windowData->setViewMode(m_currentViewMode == VIEWMODE_TYPE_FULLSCREEN,
                              backbutton);
}

void WrtClient::sendLaunchStatusToPoolServer(LaunchStatus status)
{
    if (s_processPoolDisabled) {
        return;
    }
    if( s_clientFd == -1 ){
        LogDebug("Does not connected with pool server");
        return;
    }

    int ret = 0;
    int value = static_cast<int>(status);
    ret = send(s_clientFd, &value, sizeof(int), 0);
    if (ret == -1 || ret != sizeof(int)) {
        LogError("failed to send status : " << value);
    } else {
        LogDebug("sent status to pool server : " << value);
    }

    if (s_clientFd != -1) {
        close(s_clientFd);
        s_clientFd = -1;
    }
}

void WrtClient::setTimeoutForLaunchDelayedStatus()
{
    LogDebug("enter");

    if (s_processPoolDisabled) {
        return;
    }

    static int s_isFirstAccessed = true;
    if (!s_isFirstAccessed) {
        // this function can be access only one time
        return;
    }

    s_isFirstAccessed = false;

    // start launch status timer for responding to pool server
    m_processPoolLaunchDelayedTimer =
        ecore_timer_add(
                s_processPoolLaunchDelayedWaitTime,
                processPoolLaunchDelayedTimerCallback, this);
}

void WrtClient::setTimeoutForLaunchFinishedStatus()
{
    LogDebug("enter");

    if (s_processPoolDisabled) {
        return;
    }

    static int s_isFirstAccessed = true;
    if (!s_isFirstAccessed) {
        // this function can be access only one time
        return;
    }

    s_isFirstAccessed = false;

    // start launch status timer for responding to pool server
    m_processPoolLaunchFinishedTimer =
        ecore_timer_add(
                s_processPoolLaunchFinishedWaitTime,
                processPoolLaunchFinishedTimerCallback, this);
}

void WrtClient::setTimeoutFirstFrameRenderedDelay()
{
    LogDebug("enter");

    static int s_isFirstAccessed = true;
    if (!s_isFirstAccessed) {
        // this function can be access only one time
        return;
    }
    s_isFirstAccessed = false;

    m_firstFrameRenderedDelayTimer =
        ecore_timer_add(
                s_firstFrameRenderedDelayWaitTime,
                firstFrameRenderedDelayTimerCallback, this);
}

void WrtClient::unsetTimeout(Ecore_Timer** timer)
{
    LogDebug("enter");

    if (!timer || !(*timer)) {
        return;
    }

    ecore_timer_del(*timer);
    *timer = NULL;
}

Eina_Bool WrtClient::naviframeBackButtonCallback(void* data,
                                                 Elm_Object_Item* /*it*/)
{
    LogDebug("BackButtonCallback");
    Assert(data);

    WrtClient* This = static_cast<WrtClient*>(data);
    This->m_widget->Backward();
    return EINA_FALSE;
}

int WrtClient::appcoreLowMemoryCallback(void* /*data*/)
{
    LogDebug("appcoreLowMemoryCallback");
    //WrtClient* This = static_cast<WrtClient*>(data);

    // TODO call RunnableWidgetObject API regarding low memory
    // The API should be implemented

    // temporary solution because we have no way to get ewk_context from runnable object.
    if (s_preparedEwkContext)
    {
        ewk_context_resource_cache_clear(s_preparedEwkContext);
        ewk_context_notify_low_memory(s_preparedEwkContext);
    }

    return 0;
}

void WrtClient::setInitialViewMode(void)
{
    Assert(m_dao);
    WrtDB::WindowModeList windowModes = m_dao->getWindowModes();
    FOREACH(it, windowModes) {
        std::string viewMode = DPL::ToUTF8String(*it);
        switch(viewMode[0]) {
            case 'f':
                if (viewMode == VIEWMODE_TYPE_FULLSCREEN) {
                    m_initialViewMode = viewMode;
                    m_currentViewMode = m_initialViewMode;
                    break;
                }
                break;
            case 'm':
                if (viewMode == VIEWMODE_TYPE_MAXIMIZED) {
                    m_initialViewMode = viewMode;
                    m_currentViewMode = m_initialViewMode;
                    break;
                }
                break;
            case 'w':
                if (viewMode == VIEWMODE_TYPE_WINDOWED) {
                    m_initialViewMode = viewMode;
                    m_currentViewMode = m_initialViewMode;
                    break;
                }
                break;
            default:
                break;
        }
    }
}

#ifdef ORIENTATION_ENABLED
void WrtClient::initWindowOrientation(void)
{
    Assert(m_windowData);

    m_orientationSupport.reset(
            new ClientModule::OrientationSupport(m_windowData, m_widget));
    m_orientationSupport->setInitialWindowOrientation(
            m_settingList->getRotationValue());
    if (!m_orientationSupport->setAutoRotation()) {
        LogError("Fail to set rotation callback");
    } else {
        m_orientationSupport->registerRotationCallback(autoRotationCallback,
                                                       this);
    }
}

void WrtClient::deinitWindowOrientation(void)
{
    Assert(m_orientationSupport);

    m_orientationSupport->unregisterRotationCallbacks();
    m_orientationSupport.reset();
}

void WrtClient::setWindowOrientation(int angle)
{
    Assert(m_windowData);
    m_windowData->setOrientation(angle);
}
#endif

void WrtClient::keyCallback(Evas_Object* obj, void* eventInfo)
{
    Ea_Callback_Type keyType = static_cast<Ea_Callback_Type>(reinterpret_cast<int>(eventInfo));

    // TODO: Remove keycallback when application moves to suspend
    if (m_widgetState == WidgetState_Suspended) {
        // Key event is faster than resume event
        _W("Application is not ready to handle key");
        return;
    }
    if (m_settingList->getBackButtonPresence() == BackButton_Enable
        || m_initialViewMode == VIEWMODE_TYPE_WINDOWED)
    {
        // windowed UX - hosted application
        if (keyType == EA_CALLBACK_BACK) {
            if (m_isWebkitFullscreen) {
                ewk_view_fullscreen_exit(obj);
            } else {
                m_widget->Backward();
            }
        } else if (keyType == EA_CALLBACK_MORE) {
            // UX isn't confirmed
        }
    }
}

void WrtClient::consoleMessageCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);

    static bool logEnable = (getenv(WRT_CONSOLE_LOG_ENABLE) != NULL);

#ifdef TIZEN_RELEASE_TYPE_ENG
    // override logEnable value to true
    logEnable = TRUE;
#endif

    if (m_debugMode || logEnable) {
        Assert(eventInfo);
        Ewk_Console_Message* consoleMessage = static_cast<Ewk_Console_Message*>(eventInfo);

        std::stringstream buf;
        unsigned int lineNumber = ewk_console_message_line_get(consoleMessage);
        const char* text = ewk_console_message_text_get(consoleMessage);
        const char* source = ewk_console_message_source_get(consoleMessage);
        if (lineNumber) {
            buf << source << ":";
            buf << lineNumber << ":";
        }
        buf << text;

        ConsoleLogLevel level;
        switch (ewk_console_message_level_get(consoleMessage)) {
        case EWK_CONSOLE_MESSAGE_LEVEL_TIP:
        case EWK_CONSOLE_MESSAGE_LEVEL_LOG:
        case EWK_CONSOLE_MESSAGE_LEVEL_DEBUG:
            level = ConsoleLogLevel::Debug;
            break;
        case EWK_CONSOLE_MESSAGE_LEVEL_WARNING:
            level = ConsoleLogLevel::Warning;
            break;
        case EWK_CONSOLE_MESSAGE_LEVEL_ERROR:
            level = ConsoleLogLevel::Error;
            break;
        default:
            level = ConsoleLogLevel::Debug;
            break;
        }
        ClientModule::IDESupport::consoleMessage(
            ewk_console_message_level_get(consoleMessage),
            "%s",
            buf.str().c_str());
    }
}

#ifdef ORIENTATION_ENABLED
void WrtClient::rotatePreparedCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

#if USE(WEBKIT_MANUAL_ROTATION)
    _D("Rotate by webkit");
    m_orientationSupport->setRotationDone();
#endif
}
#endif

void WrtClient::setLayout(Evas_Object* webview)
{
    LogDebug("add new webkit buffer to window");
    Assert(webview);
    m_windowData->setWebview(webview);
    evas_object_show(webview);
}

void WrtClient::unsetLayout(Evas_Object* webview)
{
    LogDebug("remove current webkit buffer from window");
    Assert(webview);
    evas_object_hide(webview);
    m_windowData->unsetWebview();
}

void WrtClient::shutdownStep()
{
    LogDebug("Closing Wrt connection ...");

    // unset timers before process shutdownStep.
    unsetTimeout(&m_processPoolLaunchFinishedTimer);
    unsetTimeout(&m_processPoolLaunchDelayedTimer);
    unsetTimeout(&m_firstFrameRenderedDelayTimer);

    if (m_widget && m_widgetState) {
        m_widgetState = WidgetState_Stopped;
        // (un)focusCallback MUST be detached before hiding widget starts
        m_windowData->smartCallbackDel(Layer::FOCUS,
                                       "focused",
                                       focusedCallback);
        m_windowData->smartCallbackDel(Layer::FOCUS,
                                       "unfocused",
                                       unfocusedCallback);
        m_widget->Hide();
        // AutoRotation use m_widget pointer internally.
        // It must be unset before m_widget is released.
        m_submodeSupport->deinitialize();
#ifdef ORIENTATION_ENABLED
        deinitWindowOrientation();
#endif
        m_widget.reset();
        ewk_object_unref(s_preparedEwkContext);
        WRT::CoreModuleSingleton::Instance().Terminate();
    }

    m_initialized = false;
    m_windowData.reset();
    Quit();
}

#ifdef ORIENTATION_ENABLED
void WrtClient::autoRotationCallback(void* data,
                                     Evas_Object* obj,
                                     void* event)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(event);
    LogDebug("entered");
    Assert(data);
    WrtClient* This = static_cast<WrtClient*>(data);
    // Splash screen
    if (This->m_splashScreen && This->m_splashScreen->isShowing()) {
        This->m_splashScreen->resizeSplashScreen();
    }

    This->rotatePrepared(This->m_widget->GetCurrentWebview());
}

void WrtClient::initialOrientationCheckCallback(void* data, Evas_Object* obj, void* event)
{
    _D("entered");

    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(event);

    Assert(data);
    WrtClient* This = static_cast<WrtClient*>(data);
    if (This->m_orientationSupport->isOrientationPrepared(This->m_settingList->getRotationValue())) {
        _D("initial orientation is ready");
        This->m_orientationSupport->unregisterRotationCallback(initialOrientationCheckCallback);
        This->DPL::Event::ControllerEventHandler<NextStepEvent>::PostEvent(NextStepEvent());
    }
}
#endif

void WrtClient::focusedCallback(void* data,
                                Evas_Object* /*obj*/,
                                void* /*eventInfo*/)
{
    LogDebug("entered");
    Assert(data);
    WrtClient* This = static_cast<WrtClient*>(data);
    elm_object_focus_set(This->m_widget->GetCurrentWebview(), EINA_TRUE);
}

void WrtClient::unfocusedCallback(void* data,
                                Evas_Object* /*obj*/,
                                void* /*eventInfo*/)
{
    LogDebug("entered");
    Assert(data);
    WrtClient* This = static_cast<WrtClient*>(data);
    elm_object_focus_set(This->m_widget->GetCurrentWebview(), EINA_FALSE);
}

static void _timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}

void WrtClient::fireLanguageChangePopup(float delay){
    Evas_Object *popup;
    popup = elm_popup_add(m_windowData->getEvasObject(Layer::WINDOW));
    elm_object_style_set(popup, "toast");
    elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup,"elm.text", WRT_BODY_LOADING_ING);
    evas_object_smart_callback_add(popup, "timeout", _timeout_cb, NULL);
    elm_popup_timeout_set(popup, delay);
    evas_object_show(popup);
}

int WrtClient::languageChangedCallback(void *data, void *tmp)
{
    LogDebug("Language Changed");
    if (!data) {
        return 0;
    }
    WrtClient* wrtClient = static_cast<WrtClient*>(data);
    if (!(wrtClient->m_dao)) {
        return 0;
    }

    // reset function fetches system locales and recreates language tags
    LanguageTagsProviderSingleton::Instance().resetLanguageTags();
    // widget default locales are added to language tags below
    DPL::OptionalString defloc = wrtClient->m_dao->getDefaultlocale();
    if (!!defloc) {
        LanguageTagsProviderSingleton::Instance().addWidgetDefaultLocales(
            *defloc);
    }

    if (wrtClient->m_launched &&
        wrtClient->m_widgetState != WidgetState_Stopped)
    {
        AppCategory type = wrtClient->checkAppCategory();

        if (wrtClient->m_widgetState == WidgetState_Running || type == APP_CATEGORY_IDLE_CLOCK) {
            // 1. In case of IDLE_CLOCK app reload page
            // 2. Running app(foreground) reload page
            wrtClient->fireLanguageChangePopup(2.0);
            wrtClient->m_widget->ReloadStartPage();
        } else if (wrtClient->m_widgetState == WidgetState_Suspended) {
            // 3. Suspended app(background) exit
            elm_exit();
        }
    }
    return 0;
}

Eina_Bool processPoolFdCallback(void* /*data*/, Ecore_Fd_Handler *handler)
{
    int fd = ecore_main_fd_handler_fd_get(handler);

    if (fd == -1)
    {
        LogDebug("ECORE_FD_GET");
        exit(-1);
    }

    if (ecore_main_fd_handler_active_get(handler, ECORE_FD_ERROR))
    {
        LogDebug("ECORE_FD_ERROR");
        close(fd);
        exit(-1);
    }

    if (ecore_main_fd_handler_active_get(handler, ECORE_FD_READ))
    {
        LogDebug("ECORE_FD_READ");
        app_pkt_t* pkt = (app_pkt_t*) malloc(sizeof(char) * AUL_SOCK_MAXBUFF);
        memset(pkt, 0, AUL_SOCK_MAXBUFF);

        int recv_ret = recv(fd, pkt, AUL_SOCK_MAXBUFF, 0);

        if (recv_ret == -1)
        {
            LogDebug("recv error!");
            exit(-1);
        }
        ecore_main_fd_handler_del(handler);

        LogDebug("recv_ret : " << recv_ret << ", pkt->len : " << pkt->len);
        process_pool_launchpad_main_loop(pkt, app_argv[0], &app_argc, &app_argv);
        free(pkt);

        // escape from blocked position of process pool
        ecore_main_loop_quit();
    }

    return ECORE_CALLBACK_CANCEL;
}

Eina_Bool WrtClient::processPoolLaunchDelayedTimerCallback(void* data)
{
    LogDebug("enter");
    WrtClient* This = static_cast<WrtClient*>(data);
    This->sendLaunchStatusToPoolServer(LAUNCH_STATUS_DELAYED);
    This->m_processPoolLaunchDelayedTimer = NULL;
    return ECORE_CALLBACK_CANCEL;
}

Eina_Bool WrtClient::processPoolLaunchFinishedTimerCallback(void* data)
{
    LogDebug("enter");
    WrtClient* This = static_cast<WrtClient*>(data);
    This->sendLaunchStatusToPoolServer(LAUNCH_STATUS_FINISHED);
    This->m_processPoolLaunchFinishedTimer = NULL;
    return ECORE_CALLBACK_CANCEL;
}

Eina_Bool WrtClient::firstFrameRenderedDelayTimerCallback(void* data)
{
    LogDebug("enter");
    WrtClient* This = static_cast<WrtClient*>(data);

    // window should be showed for more fast interation with user
    if (This->m_windowData) {
        evas_object_show(This->m_windowData->getEvasObject(Layer::WINDOW));
    }
    This->m_firstFrameRenderedDelayTimer = NULL;
    return ECORE_CALLBACK_CANCEL;
}

void WrtClient::Quit()
{
    ewk_shutdown();
    DPL::Application::Quit();
}

static void vconf_changed_handler(keynode_t* /*key*/, void* /*data*/)
{
    LogDebug("VCONFKEY_LANGSET vconf-key was changed!");

    // When system language is changed, the candidate process will be created again.
    exit(-1);
}

// appcore does not send low memory event to remove sluggish issue.
// Add callback to vconf to get low memory event.
static void vconf_lowmem_handler(keynode_t* key, void* data)
{
    LogDebug("VCONFKEY_SYSMAN_LOW_MEMORY vconf-key was changed!");

	int val = vconf_keynode_get_int(key);

	if (val < VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING)
	{
        return;
	}

    if (s_preparedEwkContext)
    {
        LogDebug("call resource cache clear!");
        ewk_context_resource_cache_clear(s_preparedEwkContext);
        ewk_context_notify_low_memory(s_preparedEwkContext);
    }

    return;
}

void set_env()
{
#ifndef TIZEN_PUBLIC
    setenv("COREGL_FASTPATH", "1", 1);
#endif
    setenv("CAIRO_GL_COMPOSITOR", "msaa", 1);
    setenv("CAIRO_GL_LAZY_FLUSHING", "yes", 1);
    setenv("ELM_IMAGE_CACHE", "0", 1);

    char * evas_socket_engine = getenv("WRT_ECORE_EVAS_EXTN_SOCKET_ENGINE");
    if( evas_socket_engine != NULL )
        setenv("ECORE_EVAS_EXTN_SOCKET_ENGINE", evas_socket_engine, 1);
}

int WrtClient::checkAppCategoryCallback(const char *name, void* result)
{
    if (!strcmp(name, CATEGORY_IDLE_CLOCK) || !strcmp(name, CATEGORY_WEARABLE_CLOCK)) {
        LogDebug("idle clock: " << name);
        *(static_cast<AppCategory*>(result)) = APP_CATEGORY_IDLE_CLOCK;
        return -1;
    }

    return 0;
}

AppCategory WrtClient::checkAppCategory()
{
    int ret = 0;
    AppCategory result = APP_CATEGORY_NORMAL;
    pkgmgrinfo_appinfo_h handle;
    ret = pkgmgrinfo_appinfo_get_appinfo(m_tizenId.c_str(), &handle);
    if (ret != PMINFO_R_OK) {
        return result;
    }
    ret = pkgmgrinfo_appinfo_foreach_category(
            handle, checkAppCategoryCallback, (void*) &result);
    if (ret != PMINFO_R_OK) {
        pkgmgrinfo_appinfo_destroy_appinfo(handle);
        return result;
    }
    pkgmgrinfo_appinfo_destroy_appinfo(handle);
    return result;
}

int main(int argc,
         char *argv[])
{
    // process pool - store arg's value
    app_argc = argc;
    app_argv = argv;

    // check process pool feature
    if (getenv("WRT_PROCESS_POOL_DISABLE")) {
        s_processPoolDisabled = true;
    }

    UNHANDLED_EXCEPTION_HANDLER_BEGIN
    {
        ADD_PROFILING_POINT("main-entered", "point");

        // Set log tagging
        DPL::Log::LogSystemSingleton::Instance().SetTag("WRT");

        // Set environment variables
        set_env();

        if (argc > 1 && argv[1] != NULL && !strcmp(argv[1], "-d"))
        {
            LogDebug("Entered dummy process mode");
            sprintf(argv[0], "%s", DUMMY_PROCESS_PATH);

            // Set 'root' home directory
            setenv(HOME, ROOT_HOME_PATH, 1);

            LogDebug("Prepare ewk_context");
            appcore_set_i18n("wrt-client", NULL);
            ewk_set_arguments(argc, argv);
            setenv("WRT_LAUNCHING_PERFORMANCE", "1", 1);
            s_preparedEwkContext = ewk_context_new_with_injected_bundle_path(BUNDLE_PATH);

            if (s_preparedEwkContext == NULL)
            {
                LogDebug("Creating webkit context was failed!");
                exit(-1);
            }

            // register language changed callback
            vconf_notify_key_changed(VCONFKEY_LANGSET, vconf_changed_handler, NULL);

            LogDebug("Prepare window_data");
            // Temporarily change HOME path to app
            // This change is needed for getting elementary profile
            // /opt/home/app/.elementary/config/mobile/base.cfg
            const char* backupEnv = getenv(HOME);
            if (!backupEnv) {
                // If getenv return "NULL", set empty string
                backupEnv = "";
            }
            setenv(HOME, APP_HOME_PATH, 1);
            LogDebug("elm_init()");
            elm_init(argc, argv);
            setenv(HOME, backupEnv, 1);

            LogDebug("WindowData()");
           s_preparedWindowData = new WindowData(
                    APP_CATEGORY_NORMAL,
                    static_cast<unsigned long>(getpid()));
            s_clientFd = __connect_process_pool_server();
            if (s_clientFd == -1) {
                LogDebug("Connecting process_pool_server was failed!");
            }
            Ecore_Fd_Handler* fdHandler =
                ecore_main_fd_handler_add(
                        s_clientFd,
                        static_cast<Ecore_Fd_Handler_Flags>(ECORE_FD_READ|ECORE_FD_ERROR),
                        processPoolFdCallback, NULL, NULL, NULL);

            if (fdHandler == NULL)
            {
                LogDebug("fd_handler is NULL");
                exit(-1);
            }

            setpriority(PRIO_PROCESS, 0, 0);

            LogDebug("dummy process temporary main loop begin");
            ecore_main_loop_begin();
            LogDebug("dummy process temporary main loop end");

            // deregister language changed callback
            vconf_ignore_key_changed(VCONFKEY_LANGSET, vconf_changed_handler);

            std::string tizenId =
                ClientModule::CommandLineParser::getTizenId(argc, argv);

            // CapabilitySupport
            std::string pluginProcessPath =
                ClientModule::SecuritySupport::getPluginProcessSoftLinkPath(tizenId);
            setenv("PLUGIN_PROCESS_EXECUTABLE_PATH_FOR_PROCESS_POOL",
                    pluginProcessPath.c_str(), 1);
            LogDebug("PLUGIN_PROCESS_EXECUTABLE_PATH_FOR_PROCESS_POOL = " <<
                     pluginProcessPath);

            ewk_context_message_post_to_injected_bundle(
                s_preparedEwkContext,
                MESSAGE_NAME_INITIALIZE,
                tizenId.c_str());

        }
        else
        {
            // This code is to fork a web process without exec.
            std::string tizenId =
                ClientModule::CommandLineParser::getTizenId(argc, argv);

            if (!tizenId.empty()) {
                if (UID_ROOT == getuid()) {
                    // Drop root permission
                    // Only launch web application by console command case has root permission
                    if (!ClientModule::SecuritySupport::setAppPrivilege(tizenId)) {
                        LogError("Fail to set app privilege : [" << tizenId << "]");
                        exit(-1);
                    }
                }

                LogDebug("Launching by fork mode");
                // Language env setup
                appcore_set_i18n("wrt-client", NULL);
                ewk_set_arguments(argc, argv);
                setenv("WRT_LAUNCHING_PERFORMANCE", "1", 1);

                // CapabilitySupport
                std::string pluginProcessPath =
                    ClientModule::SecuritySupport::getPluginProcessSoftLinkPath(tizenId);
                setenv("PLUGIN_PROCESS_EXECUTABLE_PATH",
                       pluginProcessPath.c_str(), 1);
                LogDebug("PLUGIN_PROCESS_EXECUTABLE_PATH = " <<
                         pluginProcessPath);

                s_preparedEwkContext = ewk_context_new_with_injected_bundle_path(
                        BUNDLE_PATH);

                if (s_preparedEwkContext == NULL)
                {
                    LogDebug("Creating webkit context was failed!");
                    Wrt::Popup::PopupInvoker().showInfo("Error", "Creating webkit context was failed.", "OK");
                    exit(-1);
                }

                // plugin init
                ewk_context_message_post_to_injected_bundle(
                    s_preparedEwkContext,
                    MESSAGE_NAME_INITIALIZE,
                    tizenId.c_str());
            }
        }

        // Output on stdout will be flushed after every newline character,
        // even if it is redirected to a pipe. This is useful for running
        // from a script and parsing output.
        // (Standard behavior of stdlib is to use full buffering when
        // redirected to a pipe, which means even after an end of line
        // the output may not be flushed).
        setlinebuf(stdout);

        WrtClient app(app_argc, app_argv);

        ADD_PROFILING_POINT("Before appExec", "point");
        int ret = app.Exec();
        LogDebug("App returned: " << ret);
        ret = app.getReturnStatus();
        LogDebug("WrtClient returned: " << ret);
        elm_shutdown();
        return ret;
    }
    UNHANDLED_EXCEPTION_HANDLER_END
}
