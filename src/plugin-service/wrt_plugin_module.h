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
 * @file    wrt_plugin_module.h
 * @author  Lukasz Marek (l.marek@samgsung.com)
 * @author  Yunchan Cho (yunchan.cho@samgsung.com)
 * @version
 * @brief
 */

#ifndef WRT_SRC_PLUGIN_SERVICE_PLUGIN_MODULE_H_
#define WRT_SRC_PLUGIN_SERVICE_PLUGIN_MODULE_H_

#include <cstddef>
#include <js_overlay_types.h>

extern "C" {
typedef struct OpaqueJSContext* JSGlobalContextRef;
}

namespace PluginModule {
//forward declaration
void init(int widgetHandle);
void start(int widgetHandle,
           JSGlobalContextRef context,
           double scale,
           const char* encodedBundle,
           const char* theme);
void stop(JSGlobalContextRef context);
void shutdown();
void setCustomProperties(JSGlobalContextRef context,
                         double scale,
                         const char* encodedBundle,
                         const char* theme);
void dispatchJavaScriptEvent(JSGlobalContextRef context,
                             WrtPlugins::W3C::CustomEventType eventType,
                             void* data);
void loadFrame(JSGlobalContextRef context);
void unloadFrame(JSGlobalContextRef context);
} // PluginModule

#endif
