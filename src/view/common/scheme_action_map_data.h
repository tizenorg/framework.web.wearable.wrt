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
 * @file       scheme_action_map_data.h
 * @author     Tae-Jeong Lee (taejeong.lee@samsung.com)
 * @version    1.0
 */

#include "scheme_action_map.h"

#include <dpl/platform.h>

#include <scheme.h>

namespace ViewModule {
namespace {
enum UriAction {
    URI_ACTION_WRT,       // load in WRT
    URI_ACTION_APPSVC,    // launch in APPSVC
    URI_ACTION_VIDEO,     // launch in VIDEO player
    URI_ACTION_ERROR
};

/**
 * WS-1501 - No top-level window navigation outside the widget. Deafult browser
 *           should be used instead. Frames/iframes are allowed to navigate.
 *           This requirement can't be applied to tizen hosted app as in its
 *           case the whole widget is "outside".
 *
 * WS-1502 - When calling window.open() with scheme HTTP/HTTPS and target
 *           attribute set to "_blank" WRT should open default browser. At the
 *           moment we can't distinguish target attributes, therefore all new
 *           windows are opened in the browser regardless of the attribute (the
 *           value "_new" is also treated this way). Tizen won't satisfy this
 *           requirement. It should open new windows in WRT.
 *
 * Video - YOUTUBE and RSTP are handled by video player.
 *
 * File scheme - FILE scheme has to be handled by WRT
 *
 * WS-1510/20/30/40/50 - All remaining cases are always handled by appsvc
 */

// TIZEN
const UriAction g_tizenActionMap[Scheme::COUNT][SchemeActionMap::COUNT] = {
    //  TOP_LEVEL           FRAME_LEVEL         NEW_WINDOW
    { URI_ACTION_WRT, URI_ACTION_WRT, URI_ACTION_WRT },           // FILE
    { URI_ACTION_APPSVC, URI_ACTION_APPSVC, URI_ACTION_APPSVC },  // SMS
    { URI_ACTION_APPSVC, URI_ACTION_APPSVC, URI_ACTION_APPSVC },  // SMSTO
    { URI_ACTION_APPSVC, URI_ACTION_APPSVC, URI_ACTION_APPSVC },  // MMSTO
    { URI_ACTION_APPSVC, URI_ACTION_APPSVC, URI_ACTION_APPSVC },  // MAILTO
    { URI_ACTION_WRT, URI_ACTION_WRT, URI_ACTION_WRT },        // DATA
    { URI_ACTION_APPSVC, URI_ACTION_APPSVC, URI_ACTION_APPSVC },  // TEL
    { URI_ACTION_WRT, URI_ACTION_WRT, URI_ACTION_WRT },           // HTTP
    { URI_ACTION_WRT, URI_ACTION_WRT, URI_ACTION_WRT },           // HTTPS
    { URI_ACTION_WRT, URI_ACTION_WRT, URI_ACTION_WRT },           // WIDGET
#if ENABLE(APP_SCHEME)
    { URI_ACTION_WRT, URI_ACTION_WRT, URI_ACTION_WRT },           // APP
#endif
    { URI_ACTION_VIDEO, URI_ACTION_VIDEO, URI_ACTION_VIDEO }      // RTSP
};

}
}
