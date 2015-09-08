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
 * @file    localization_setting.cpp
 * @author  Soyoung Kim (sy037.kim@samsung.com)
 * @version 1.0
 * @brief   Localization setting implementation
 */

#include "localization_setting.h"
#include <dpl/log/log.h>

#include <appcore-efl.h>

namespace LocalizationSetting {
void SetLanguageChangedCallback(LanguageChangedCallback cb, void *data)
{
    LogDebug("Set language changed callback");

    appcore_set_event_callback(
        APPCORE_EVENT_LANG_CHANGE,
        cb, data);
}
} //Namespace LocalizationSetting
