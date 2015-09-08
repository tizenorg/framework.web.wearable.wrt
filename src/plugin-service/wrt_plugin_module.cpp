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
 * @file    wrt_plugin_module.cpp
 * @author  Lukasz Marek (l.marek@samgsung.com)
 * @author  Yunchan Cho (yunchan.cho@samgsung.com)
 * @version
 * @brief
 */

#include "wrt_plugin_module.h"

#include <dpl/log/secure_log.h>
#include <dpl/string.h>
#include <dpl/assert.h>
#include <plugin_logic.h>
#include <js_overlay_types.h>

namespace PluginModule {
void init(int widgetHandle)
{
    _D("called");
    PluginLogicSingleton::Instance().initSession(widgetHandle);
}

void start(int widgetHandle,
           JSGlobalContextRef context,
           double scale,
           const char *encodedBundle,
           const char *theme)
{
    PluginLogicSingleton::Instance().startSession(widgetHandle,
                                                  context,
                                                  scale,
                                                  encodedBundle,
                                                  theme);
}

void shutdown()
{
    _D("called");
    PluginLogicSingleton::Instance().performLibrariesUnload();
}

void stop(JSGlobalContextRef context)
{
    _D("called");
    PluginLogicSingleton::Instance().stopSession(context);
}

void setCustomProperties(JSGlobalContextRef context,
                         double scale,
                         const char* encodedBundle,
                         const char* theme)
{
    _D("called");
    PluginLogicSingleton::Instance().setCustomProperties(context,
                                                         scale,
                                                         encodedBundle,
                                                         theme);
}

void dispatchJavaScriptEvent(JSGlobalContextRef context,
                             WrtPlugins::W3C::CustomEventType eventType,
                             void* data)
{
    _D("called");
    PluginLogicSingleton::Instance().dispatchJavaScriptEvent(context,
                                                             eventType,
                                                             data);
}

void loadFrame(JSGlobalContextRef context)
{
    _D("load frame into web page (context: %p)", context);
    PluginLogicSingleton::Instance().loadFrame(context);
}

void unloadFrame(JSGlobalContextRef context)
{
    _D("unload frame from web page (context: %p)", context);
    PluginLogicSingleton::Instance().unloadFrame(context);
}
} // PluginModule
