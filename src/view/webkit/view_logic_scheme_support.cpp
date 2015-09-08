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
/*
 * @file       view_logic_scheme_support.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "view_logic_scheme_support.h"

#include <dpl/assert.h>
#include <dpl/log/secure_log.h>
#include <common/scheme_action_map.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace ViewModule {
namespace {
char const * const TIZEN_SCHEME = "tizen";
char const * const TIZEN_EXIT = "tizen://exit";
char const * const TIZEN_HIDE = "tizen://hide";
char const * const TIZEN_CHANGE_USERAGNET = "tizen://changeUA";

static Eina_Bool exitAppIdlerCallback(void* /*data*/)
{
    elm_exit();
    return ECORE_CALLBACK_CANCEL;
}
} // namespace

bool SchemeSupport::HandleTizenScheme(const char* uri,
                                      Evas_Object* window,
                                      Evas_Object* wkView)
{
    if (!uri) {
        _W("Empty uri");
        return false;
    }
    if (!window) {
        _W("Empty window data");
        return false;
    }

    if (strncmp(uri, TIZEN_EXIT, strlen(TIZEN_EXIT)) == 0) {
        _D("%s : exit", uri);
        ecore_idle_enterer_before_add(exitAppIdlerCallback, NULL);
        return true;
    } else if (strncmp(uri, TIZEN_HIDE, strlen(TIZEN_HIDE)) == 0) {
        _D("%s : hide", uri);
        elm_win_lower(window);
        return true;
    } else if (strncmp(uri,
                       TIZEN_CHANGE_USERAGNET,
                       strlen(TIZEN_CHANGE_USERAGNET)) == 0)
    {
        _D("%s : change UA", uri);
        const char* userAgentString = strstr(uri, "=");
        if (NULL == userAgentString) {
            _W("UA string is NULL");
        } else {
            userAgentString++;
            _D("Setting custom user agent as: %s", userAgentString);
            ewk_view_user_agent_set(wkView, userAgentString);
        }
        return true;
    }
    return false;
}

bool SchemeSupport::filterURIByScheme(Ewk_Policy_Decision* policyDecision,
                                      bool newWindow,
                                      Evas_Object* window,
                                      Evas_Object* wkView)
{
    _D("called");
    Assert(policyDecision);

    const char* url = ewk_policy_decision_url_get(policyDecision);
    if (NULL == url) {
        // URI is null
        // There is no reason, security by scheme block null uri
        return true;
    }

    bool mainFrame =
        ewk_frame_is_main_frame(ewk_policy_decision_frame_get(policyDecision));

    using namespace ViewModule::SchemeActionMap;
    if (HandleTizenScheme(url, window, wkView)) {
        _D("Scheme is tizen scheme");
        return false;
    }

    NavigationContext ctx;
    ctx = (mainFrame ? TOP_LEVEL : FRAME_LEVEL);
    if (newWindow) {
        // In case of Tizen,
        // policy of new window should be applied,
        // regardless level of this frame
        ctx = NEW_WINDOW;
    }

    return SchemeActionMap::HandleUri(url, ctx);
}
} // namespace ViewModule

