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
 * @file    view_logic_custom_header_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @brief   Implementation file of CustomHeaderSupport API used by ViewLogic
 */

#include "view_logic_custom_header_support.h"

#include <vconf.h>
#include <vconf-keys.h>

#include <string>
#include <dpl/log/log.h>

namespace {
const std::string LANGUAGE_EN = "en";
};

namespace ViewModule {
namespace CustomHeaderSupport {
std::string getValueByField(const std::string &field)
{
    LogDebug("Field : " << field);
    std::string ret;

    if (field == ACCEPT_LANGUAGE) {
        char *systemLanguageSet = NULL;
        systemLanguageSet = vconf_get_str(VCONFKEY_LANGSET);
        LogDebug("system language = [" << systemLanguageSet << "]");

        if (!systemLanguageSet) {
            LogError("Failed to get VCONFKEY_LANGSET. set as English");
            ret.append(LANGUAGE_EN);
        } else {
            // copy first 2bytes of language set (en, ko, po)
            ret.append(systemLanguageSet, 2);
        }

        if (systemLanguageSet) {
            free(systemLanguageSet);
        }
    } else {
        LogError("Wrong field is passed");
        Assert(false);
    }

    return ret;
}
} // namespace CustomHeaderSupport
} // namespace ViewModule
