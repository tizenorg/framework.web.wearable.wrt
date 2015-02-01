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
 * @file    view_logic_keys_support.cpp
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @brief   Implementation file of SecuritySupport API used by ViewLogic
 */

#include "view_logic_security_support.h"
#include <string>
#include <dpl/string.h>
#include <dpl/log/log.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <ace_api_client.h>
#include <dpl/utils/warp_iri.h>
#include <widget_data_types.h>

namespace ViewModule {
namespace SecuritySupport {
namespace {
const char *GEOLOCATION_DEV_CAP = "geolocation.position";
const char *GEOLOCATION_PARAM_NAME = "param:enableHighAccuracy";
const char *GEOLOCATION_PARAM_VALUE = "true";

bool simpleAceCheck(
    const DPL::String& tizenId,
    const char *devCap,
    const char *paramName,
    const char *paramValue)
{
    WrtDB::WidgetDAOReadOnly dao(tizenId);
    ace_request_t aceRequest;
    aceRequest.widget_handle = dao.getHandle();
    aceRequest.session_id = const_cast<const ace_string_t>("");
    aceRequest.feature_list.count = 0;
    aceRequest.dev_cap_list.count = 1;
    aceRequest.dev_cap_list.items = new ace_dev_cap_t[1];
    aceRequest.dev_cap_list.items[0].name =
        const_cast<ace_string_t>(devCap);

    aceRequest.dev_cap_list.items[0].param_list.count = paramName ? 1 : 0;
    aceRequest.dev_cap_list.items[0].param_list.items = NULL;

    if (paramName) {
        aceRequest.dev_cap_list.items[0].param_list.items = new ace_param_t[1];
        aceRequest.dev_cap_list.items[0].param_list.items[0].name =
            const_cast<ace_string_t>(paramName);
        aceRequest.dev_cap_list.items[0].param_list.items[0].value =
            const_cast<ace_string_t>(paramValue);
    }

    LogDebug("Making ace check with new C-API");
    ace_check_result_t result = ACE_PRIVILEGE_DENIED;
    ace_return_t ret = ace_check_access_ex(&aceRequest, &result);

    LogDebug("Result is: " << static_cast<int>(result));

    delete[] aceRequest.dev_cap_list.items[0].param_list.items;
    delete[] aceRequest.dev_cap_list.items;

    return ACE_OK == ret && ACE_ACCESS_GRANTED == result;
}
} //TODO copied from view_logic.cpp

bool geolocationACECheck(const DPL::String& tizenId, bool highAccuracy)
{
    const char *paramName = NULL;
    const char *paramValue = NULL;
    if (highAccuracy) {
        paramName = GEOLOCATION_PARAM_NAME;
        paramValue = GEOLOCATION_PARAM_VALUE;
    }
    return simpleAceCheck(
               tizenId,
               GEOLOCATION_DEV_CAP,
               paramName,
               paramValue);
}
} // namespace SecuritySupport
} //namespace ViewModule
