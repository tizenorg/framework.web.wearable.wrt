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
 * @file    view_logic_message_support.cpp
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @brief   View logic message support - implementation
 */
#include "view_logic_message_support.h"

#include <sstream>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <common/message_support.h>
#include <js_overlay_types.h>

namespace ViewLogicMessageSupport {
void init(Ewk_Context* ewkContext, const std::string& tizenId)
{
    const char* name = Message::ToInjectedBundle::INIT;
    const char* msg = tizenId.c_str();

    ewk_context_message_post_to_injected_bundle(ewkContext, name, msg);
}

void start(Ewk_Context* ewkContext,
           const DPL::String& tizenId,
           double scale,
           const char *encodedBundle,
           const char *theme)
{
    std::stringstream ssMsg;

    std::string id = DPL::ToUTF8String(tizenId);
    ssMsg << id << " ";

    ssMsg << "_" << scale << " ";

    if (encodedBundle) {
        ssMsg << "_" << encodedBundle << " ";
    } else {
        ssMsg << "null" << " ";
    }

    if (theme) {
        ssMsg << "_" << theme;
    } else {
        ssMsg << "null";
    }

    std::string msgString = ssMsg.str();

    const char* msg = msgString.c_str();
    const char* name = Message::ToInjectedBundle::START;

    ewk_context_message_post_to_injected_bundle(ewkContext, name, msg);
}

void shutdown(Ewk_Context* ewkContext)
{
    const char* name = Message::ToInjectedBundle::SHUTDOWN;
    ewk_context_message_post_to_injected_bundle(ewkContext, name, name);
}

void setCustomProperties(
    Ewk_Context* ewkContext,
    double* scale,
    const char* encodedBundle,
    const char* theme)
{
    std::stringstream ssMsg;

    if (scale) {
        ssMsg << "_" << *scale << " ";
    } else {
        ssMsg << "null" << " ";
    }

    if (encodedBundle) {
        ssMsg << "_" << encodedBundle << " ";
    } else {
        ssMsg << "null" << " ";
    }

    if (theme) {
        ssMsg << "_" << theme;
    } else {
        ssMsg << "null";
    }

    std::string msgString = ssMsg.str();

    const char* msg = msgString.c_str();
    const char* name = Message::ToInjectedBundle::SET_CUSTOM_PROPERTIES;

    ewk_context_message_post_to_injected_bundle(ewkContext, name, msg);
}

void dispatchJavaScriptEvent(
    Ewk_Context* ewkContext,
    WrtPlugins::W3C::CustomEventType eventType,
    void *data)
{
    using namespace WrtPlugins::W3C;
    std::stringstream str;
    str << eventType;

    // if needed, arguments for event should be set here
    if (eventType == SoftKeyboardChangeCustomEvent) {
        if (data) {
            SoftKeyboardChangeArgs* args =
                static_cast<SoftKeyboardChangeArgs *>(data);
            str << " " << args->state;
            str << " " << args->width;
            str << " " << args->height;
        }
    }

    std::string msgString = str.str();
    const char* msg = msgString.c_str();
    const char* name = Message::ToInjectedBundle::DISPATCH_JS_EVENT;

    ewk_context_message_post_to_injected_bundle(ewkContext, name, msg);
}

void setXwindowHandle(Ewk_Context* ewkContext, const unsigned int handle)
{
    const char* name = Message::ToInjectedBundle::SET_XWINDOW_HANDLE;
    std::stringstream ssMsg;
    ssMsg << handle;
    std::string msgString = ssMsg.str();
    const char* msg = msgString.c_str();

    ewk_context_message_post_to_injected_bundle(ewkContext, name, msg);
}

void setViewmodes(Ewk_Context* ewkContext, const char* msg)
{
    const char* name = Message::ToInjectedBundle::SET_VIEWMODES;
    ewk_context_message_post_to_injected_bundle(ewkContext, name, msg);
}
} // namespace ViewLogicMessageSupport
