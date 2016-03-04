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
 * @file    message_support.h
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @brief   definition of messages between UI Process and Injected bundle
 */
#ifndef WRT_SRC_VIEW_COMMON_MESSAGE_SUPPORT_H_
#define WRT_SRC_VIEW_COMMON_MESSAGE_SUPPORT_H_

namespace Message {

namespace ToInjectedBundle {
const char * const INIT = "ToInjectedBundle::INIT";
const char * const START = "ToInjectedBundle::START";
const char * const SHUTDOWN = "ToInjectedBundle::SHUTDOWN";
const char * const SET_CUSTOM_PROPERTIES =
    "ToInjectedBundle::SET_CUSTOM_PROPERTIES";
const char * const DISPATCH_JS_EVENT = "ToInjectedBundle::DISPATCH_JS_EVENT";
const char * const SET_XWINDOW_HANDLE = "ToInjectedBundle::SET_XWINDOW_HANDLE";
const char * const SET_VIEWMODES = "ToInjectedBundle::SET_VIEWMODES";
const char * const SET_VIEWMODES_MSGBODY_EXIT = "exit";
} // namespace ToInectedBundle

namespace ToUIProcess {
const char * const BLOCKED_URL = "ToUIProcess::BLOCKED_URL";
const char * const SEND_WEBPROCESS_PID = "ToUIProcess::SEND_WEBPROCESS_PID";
const char * const BACKGROUND_SUPPORTED = "ToUIProcess::BACKGROUND_SUPPORTED";
} // namespace ToUIProcess

namespace TizenScheme {
const char * const GET_WINDOW_HANDLE = "tizen://getWindowHandle";
const char * const CLEAR_ALL_COOKIES = "tizen://clearAllCookies";
} // namespace ToUIProcess

} //namespace BundleMessages

#endif // WRT_SRC_VIEW_COMMON_MESSAGE_SUPPORT_H_
