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
/**
 * @file        user_delegates.h
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       user delegates
 */
#ifndef USER_DELEGATES_H
#define USER_DELEGATES_H

#include <functional>
#include <memory>
#include <string>

#include <Evas.h>

namespace WRT {
typedef std::function<void(Evas_Object*)> SetWebviewCallback;
typedef std::function<void(Evas_Object*)> UnsetWebviewCallback;
typedef std::function<void(Evas_Object*, void*)> LoadProgressStartedCallback;
typedef std::function<void(Evas_Object*, void*)> LoadProgressCallback;
typedef std::function<void(Evas_Object*, void*)> LoadProgressFinishedCallback;
typedef std::function<void(Evas_Object*, void*)> LoadStartedCallback;
typedef std::function<void(Evas_Object*, void*)> LoadFinishedCallback;
typedef std::function<void(Evas_Object*, void*)> ProcessCrashedCallback;
typedef std::function<bool(Evas_Object*, void*)> PolicyNavigationDecideCallback;
typedef std::function<void(Evas_Object*, void*)> ProcessExitCallback;
typedef std::function<void(Evas_Object*, void*)> EnterFullscreenCallback;
typedef std::function<void(Evas_Object*, void*)> ExitFullscreenCallback;
typedef std::function<void(int)> OrientationLockCallback;
typedef std::function<void(Evas_Object*, void*)> KeyCallback;
typedef std::function<void(Evas_Object*, void*)> ConsoleMessageCallback;
typedef std::function<void(Evas_Object*, void*)> RotatePreparedCallback;
typedef std::function<void(Evas_Object*, void*)> EnableVideoHwOverlayCallback;
typedef std::function<void(Evas_Object*, void*)> DisableVideoHwOverlaycallback;
typedef std::function<void(Evas_Object*, void*)> PopupReplyWaitStartCallback;
typedef std::function<void(Evas_Object*, void*)> PopupReplyWaitFinishCallback;
typedef std::function<void(Evas_Object*, void*)> FrameRenderedCallback;
typedef std::function<void(const std::string&) > BlockedUrlPolicyCallback;

struct UserDelegates {
    SetWebviewCallback setWebviewCallback;
    UnsetWebviewCallback unsetWebviewCallback;
    LoadProgressStartedCallback loadProgressStartedCallback;
    LoadProgressCallback loadProgressCallback;
    LoadProgressFinishedCallback loadProgressFinishedCallback;
    LoadStartedCallback loadStartedCallback;
    LoadFinishedCallback loadFinishedCallback;
    ProcessCrashedCallback processCrashedCallback;
    PolicyNavigationDecideCallback policyNavigationDecideCallback;
    ProcessExitCallback processExitCallback;
    EnterFullscreenCallback enterFullscreenCallback;
    ExitFullscreenCallback exitFullscreenCallback;
    OrientationLockCallback orientationLockCallback;
    KeyCallback keyCallback;
    ConsoleMessageCallback consoleMessageCallback;
    RotatePreparedCallback rotatePreparedCallback;
    EnableVideoHwOverlayCallback enableVideoHwOverlayCallback;
    DisableVideoHwOverlaycallback disableVideoHwOverlayCallback;
    PopupReplyWaitStartCallback popupReplyWaitStartCallback;
    PopupReplyWaitFinishCallback popupReplyWaitFinishCallback;
    FrameRenderedCallback frameRenderedCallback;
    BlockedUrlPolicyCallback blockedUrlPolicyCallback;
};

typedef std::shared_ptr<UserDelegates> UserDelegatesPtr;
}

#endif // USER_DELEGATES_H
