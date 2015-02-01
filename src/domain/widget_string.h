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
 * @file       widget_string.h
 * @author     Jihoon Chung(jihoon.chung@samsung.com)
 * @version    0.1
 * @brief
 */

#ifndef WRT_SRC_DOMAIN_WIDGET_STRING_H_
#define WRT_SRC_DOMAIN_WIDGET_STRING_H_

#include <app.h>
#include <initializer_list>
#include <string>

#include <dpl/log/secure_log.h>

#define WRT_PS "%s"
#define WRT_PNS "%d$s"

namespace WrtText {
inline std::string replacePS(std::initializer_list<std::string> strs) {
    std::size_t size = strs.size();
    if (size <= 1 || size >= 10) {
        return std::string("");
    }

    std::initializer_list<std::string>::iterator it = strs.begin();
    std::string ret = *strs.begin();
    std::string arg = *(++it);

    // %s -> arg
    std::size_t ps = ret.find(WRT_PS);
    if (ps != std::string::npos) {
        ret.replace(ps, std::string(WRT_PS).length(), arg);
        return ret;
    }

    // %n$s -> n_arg
    std::string n = "1";
    for ( ; it != strs.end(); ++it) {
        std::string pns = WRT_PNS;
        pns.replace(1, 1, n);
        n[0]++;
        auto findit = ret.find(pns);
        if( findit == std::string::npos)
            break;
        ret.replace(findit, 4, (*it).c_str());
    }
    return ret;
}
}

#define WRT_SK_OK dgettext("wrt", "IDS_BR_SK_OK")
#define WRT_SK_YES dgettext("wrt", "IDS_BR_SK_YES")
#define WRT_SK_NO dgettext("wrt", "IDS_BR_SK_NO")
#define WRT_SK_LOGIN dgettext("wrt", "IDS_BR_BUTTON_LOGIN")
#define WRT_SK_CANCEL dgettext("wrt", "IDS_BR_SK_CANCEL")
#define WRT_OPT_ALLOW dgettext("wrt", "IDS_BR_OPT_ALLOW")
#define WRT_OPT_DENY dgettext("wrt", "IDS_KA_BODY_DENY")

#define WRT_POP_USERMEDIA_PERMISSION dgettext("wrt", "IDS_JAVA_POP_ALLOW_TO_USE_MEDIA_RECORDING_Q")
#define WRT_POP_WEB_NOTIFICATION_PERMISSION dgettext("wrt", "IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_SHOW_NOTIFICATIONS")
#define WRT_POP_GEOLOCATION_PERMISSION dgettext("wrt", "IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_ACCESS_YOUR_LOCATION")
#define WRT_POP_WEB_STORAGE_PERMISSION dgettext("wrt", "IDS_BR_POP_P1SS_HP2SS_IS_ATTEMPTING_TO_STORE_A_LARGE_AMOUNT_OF_DATA_ON_YOUR_DEVICE_FOR_OFFLINE_USE")
#define WRT_POP_APPLICATION_CACHE_PERMISSION dgettext("wrt", "IDS_BR_POP_P1SS_HP2SS_IS_REQUESTING_PERMISSION_TO_STORE_DATA_ON_YOUR_DEVICE_FOR_OFFLINE_USE")
#define WRT_POP_STARTING_DOWNLOADING dgettext("wrt", "IDS_BR_POP_STARTING_DOWNLOAD_ING")
#define WRT_POP_CERTIFICATE_PERMISSION dgettext("wrt", "IDS_BR_BODY_SECURITY_CERTIFICATE_PROBLEM_MSG")
#define WRT_BODY_AUTHENTICATION_REQUIRED dgettext("wrt", "IDS_BR_BODY_DESTINATIONS_AUTHENTICATION_REQUIRED")
#define WRT_BODY_PASSWORD dgettext("wrt", "IDS_BR_BODY_PASSWORD")
#define WRT_BODY_AUTHUSERNAME dgettext("wrt", "IDS_BR_BODY_AUTHUSERNAME")
#define WRT_BODY_REMEMBER_PREFERENCE dgettext("wrt", "IDS_BR_BODY_REMEMBER_PREFERENCE")
#define WRT_POP_CLEAR_DEFAULT_SETTINGS dgettext("wrt", "IDS_ST_POP_CLEAR_DEFAULT_APP_SETTINGS_BY_GOING_TO_SETTINGS_GENERAL_MANAGE_APPLICATIONS_ALL")
#define WRT_BODY_LOADING_ING dgettext("wrt", "IDS_BR_BODY_LOADING_ING")

#endif // WRT_SRC_DOMAIN_WIDGET_STRING_H_
