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
 * @file    view_logic_message_support.h
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @brief   View logic message support - declaration
 */
#ifndef VIEW_LOGIC_MESSAGE_SUPPORT_H_
#define VIEW_LOGIC_MESSAGE_SUPPORT_H_

#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <dpl/string.h>
#include <js_overlay_types.h>

namespace ViewLogicMessageSupport {
void init(Ewk_Context* ewkContext,
          const std::string& tizenId);
void start(Ewk_Context* ewkContext,
           const DPL::String& tizenId,
           double scale,
           const char *encodedBundle,
           const char *theme);
void shutdown(Ewk_Context* ewkContext);
void setCustomProperties(
    Ewk_Context* ewkContext,
    double* scale = NULL,
    const char* encodedBundle = NULL,
    const char* theme = NULL);
void dispatchJavaScriptEvent(
    Ewk_Context* ewkContext,
    WrtPlugins::W3C::CustomEventType eventType,
    void* data);
void setXwindowHandle(Ewk_Context* ewkContext, const unsigned int handle);
void setViewmodes(Ewk_Context* ewkContext, const char* msg);
} // namespace ViewLogicMessageSupport

#endif // VIEW_LOGIC_MESSAGE_SUPPORT_H_
