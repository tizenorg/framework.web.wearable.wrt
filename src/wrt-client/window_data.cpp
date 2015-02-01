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
 * @file        window_data.cpp
 * @author      Jaroslaw Osmanski (j.osmanski@samsung.com)
 * @version     1.0
 * @brief       Window data class implementation
 */
#include "window_data.h"

#include <dpl/log/log.h>
#include <dpl/foreach.h>
#include <dpl/availability.h>

#include <efl_assist_screen_reader.h>
#include <minicontrol-provider.h>

namespace {
const unsigned int UID_ROOT = 0;
const char* const DELETE_REQUEST = "delete,request";
const char* const PROFILE_CHANGED = "profile,changed";
const char* const DESKTOP_ICON_PATH =
    "/opt/share/icons/default/small/tizenScmgz.png";
const char* const USER_DATA_KEY = "__WRT_DATA_KEY__";
const std::string DESKTOP_PROFILE("desktop");
const std::string MINICONTROL_MSG_RESUME("mc_resume");
const std::string MINICONTROL_MSG_SUSPEND("mc_pause");
const int PROGRESS_H = 10;
} // anonymous namespace

WindowData::WindowData(AppCategory category, unsigned long pid, bool manualInit) :
    m_win(NULL),
    m_bg(NULL),
    m_conformant(NULL),
    m_topLayout(NULL),
    m_naviframe(NULL),
    m_mainLayout(NULL),
    m_progressbar(NULL),
    m_initialized(false),
    m_currentViewModeFullScreen(false),
    m_category(category)
{
    m_win = createWindow(pid);

    if (!manualInit) {
        init();
    }
}

WindowData::~WindowData()
{
    LogDebug("");
    evas_object_del(m_win);
}

void WindowData::init()
{
    AssertMsg(m_win != NULL, "m_win is null");

    if (m_initialized == true) {
        LogDebug("Already initilized");
        return;
    }

    m_bg = createBg(m_win);
    evas_object_show(m_bg);
    m_conformant = createConformant(m_win);
    evas_object_show(m_conformant);
    m_topLayout = createTopLayout(m_conformant);
    evas_object_show(m_topLayout);
    m_naviframe = createNaviframe(m_topLayout);
    evas_object_show(m_naviframe);
    m_mainLayout = createMainLayout(m_naviframe);
    evas_object_show(m_mainLayout);
    m_focus = createFocus(m_mainLayout);
    evas_object_show(m_focus);
    m_progressbar = createProgressBar(m_win, m_mainLayout);
    evas_object_show(m_progressbar);

    if(m_category == APP_CATEGORY_IDLE_CLOCK){
        toggleTransparent(true);
    }

    m_initialized = true;
}

void WindowData::postInit()
{
    AssertMsg(m_win != NULL, "m_win is null");
    AssertMsg(m_initialized, "Not init");

    // postInit should called after process permission is changed to app
    if (UID_ROOT == getuid()) {
        Assert(!"Cannot do with root permission");
    }
    elm_win_indicator_mode_set(m_win, ELM_WIN_INDICATOR_SHOW);
}

bool WindowData::initScreenReaderSupport(bool isSupportAccessibility)
{
    LogDebug("called");
    AssertMsg(m_win != NULL, "x window is Null");
    return ea_screen_reader_support_set(
               m_win,
               isSupportAccessibility ? EINA_TRUE : EINA_FALSE);
}

Evas_Object* WindowData::getEvasObject(Layer layer)
{
    EvasObjectDataIt it = m_evasObjectData.find(layer);
    if (it == m_evasObjectData.end()) {
        return NULL;
    }
    return it->second;
}

void WindowData::setWebview(Evas_Object* webview)
{
    elm_object_part_content_set(m_focus, "elm.swallow.content", webview);
    elm_object_focus_set(m_focus, EINA_TRUE);
}

void WindowData::unsetWebview()
{
    elm_object_part_content_unset(m_focus, "elm.swallow.content");
}

void WindowData::smartCallbackAdd(
    Layer layer,
    const char* event,
    Evas_Smart_Cb callback,
    const void* data)
{
    Evas_Object* obj = getEvasObject(layer);
    if (!obj) {
        LogError("Fail to get Evas_Object");
        return;
    }
    evas_object_smart_callback_add(obj, event, callback, data);
    return;
}

void WindowData::smartCallbackDel(
    Layer layer,
    const char* event,
    Evas_Smart_Cb callback)
{
    Evas_Object* obj = getEvasObject(layer);
    if (!obj) {
        LogError("Fail to get Evas_Object");
        return;
    }
    evas_object_smart_callback_del(obj, event, callback);
}

void WindowData::setMiniControlCallback(const WindowData::MiniControlCallbackPtr& callback)
{
    Assert(m_win);

    m_miniControlCallback = callback;

    // set msg callback to support resume, suspend
    Evas* e = evas_object_evas_get(m_win);
    AssertMsg(e, "Fail to get Evas");
    Ecore_Evas* ee = ecore_evas_ecore_evas_get(e);
    AssertMsg(ee, "Fail to get Ecore_Evas");
    ecore_evas_data_set(ee, USER_DATA_KEY, this);
    ecore_evas_callback_msg_parent_handle_set(ee, miniControlCallback);
}

void WindowData::signalEmit(Layer layer,
                            const char* emission,
                            const char* source)
{
    Evas_Object* obj = getEvasObject(layer);
    if (!obj) {
        LogError("Fail to get Evas_Object");
        return;
    }
    edje_object_signal_emit(elm_layout_edje_get(obj), emission, source);
}

void WindowData::setViewMode(bool fullscreen, bool backbutton)
{
    LogDebug("fullscreen: " << fullscreen);
    LogDebug("backbutton: " << backbutton);

    m_currentViewModeFullScreen = fullscreen;
    toggleIndicator(fullscreen);
}

void WindowData::setOrientation(int angle)
{
    LogDebug("setOrientation");
    Assert(m_win);
    elm_win_wm_rotation_preferred_rotation_set(m_win, angle);
}

void WindowData::toggleFullscreen(bool fullscreen)
{
    toggleIndicator(fullscreen || m_currentViewModeFullScreen);
}

void WindowData::toggleTransparent(bool transparent)
{
    if (transparent) {
        edje_object_signal_emit(elm_layout_edje_get(m_mainLayout),
                                "show,transparent,signal",
                                "");
        evas_object_render_op_set(m_bg, EVAS_RENDER_COPY);
    } else {
        edje_object_signal_emit(elm_layout_edje_get(m_mainLayout),
                                "hide,transparent,signal",
                                "");
        evas_object_render_op_set(m_bg, EVAS_RENDER_BLEND);
    }
}

void WindowData::updateProgress(double value)
{
    int x, y, w, h;
    evas_object_geometry_get(m_mainLayout, &x, &y, &w, &h);
    evas_object_resize(m_progressbar,
                       static_cast<int>(w * value),
                       static_cast<int>(PROGRESS_H * elm_config_scale_get()));
}

void WindowData::setEvasObjectData(Layer layer, Evas_Object* obj)
{
    m_evasObjectData[layer] = obj;
}

Evas_Object* WindowData::createWindow(unsigned long pid)
{
    ADD_PROFILING_POINT("elm_win_add", "start");
    Evas_Object* window = NULL;
    LogDebug("Category: " << m_category);
    switch (m_category) {
    case APP_CATEGORY_IDLE_CLOCK:
        {
            std::ostringstream name;
            name << pid;
            window = minicontrol_win_add(name.str().c_str());
            elm_win_alpha_set(window, EINA_TRUE);
            evas_object_render_op_set(window, EVAS_RENDER_COPY);
            evas_object_show(window);
        }
        break;
    default:
        elm_config_preferred_engine_set("opengl_x11");
        window = elm_win_add(NULL, "wrt-widget", ELM_WIN_BASIC);
        break;
    }

    ADD_PROFILING_POINT("elm_win_add", "stop");
    ecore_x_window_prop_property_set(
        elm_win_xwindow_get(window),
        ECORE_X_ATOM_NET_WM_PID,
        ECORE_X_ATOM_CARDINAL, 32, &pid, 1);
    elm_win_conformant_set(window, EINA_TRUE);

    // use vsync of system for animator
    ecore_x_vsync_animator_tick_source_set(elm_win_xwindow_get(window));

    int w, h;
    ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
    evas_object_resize(window, w, h);
    elm_win_autodel_set(window, EINA_TRUE);
    evas_object_smart_callback_add(window,
                                   DELETE_REQUEST,
                                   winDeleteRequestCallback,
                                   this);
    evas_object_smart_callback_add(window,
                                   PROFILE_CHANGED,
                                   winProfileChangedCallback,
                                   this);
    setEvasObjectData(Layer::WINDOW, window);
    return window;
}

Evas_Object* WindowData::createBg(Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    Evas_Object* obj = evas_object_rectangle_add(evas_object_evas_get(parent));
    evas_object_color_set(obj, 0, 0, 0, 0);
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(parent, obj);
    evas_object_render_op_set(obj, EVAS_RENDER_BLEND);
    setEvasObjectData(Layer::BG, obj);
    return obj;
}

Evas_Object* WindowData::createConformant(Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    Evas_Object* obj = elm_conformant_add(parent);
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(parent, obj);
    setEvasObjectData(Layer::CONFORMANT, obj);
    return obj;
}

Evas_Object* WindowData::createTopLayout(Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    Evas_Object* obj = elm_layout_add(parent);
    elm_layout_theme_set(obj, "layout", "application", "default");
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_content_set(parent, obj);
    setEvasObjectData(Layer::TOP_LAYOUT, obj);
    return obj;
}

Evas_Object* WindowData::createNaviframe(Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    Evas_Object* obj = elm_naviframe_add(parent);
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_content_set(parent, "elm.swallow.content", obj);
    setEvasObjectData(Layer::NAVIFRAME, obj);
    return obj;
}

Evas_Object* WindowData::createMainLayout(Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    Evas_Object* obj = elm_layout_add(parent);
    elm_layout_file_set(obj, WRT_EDJ_PATH, "web-application");
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    ADD_PROFILING_POINT("elm_naviframe_item_push", "start");
    Elm_Object_Item* naviIt =
        elm_naviframe_item_push(
            parent,   // Evas_Object* obj
            NULL,     // const char* title_label
            NULL,     // Evas_Object* prev_btn
            NULL,     // Evas_Object* next_btn
            obj,      // Evas_Object* content
            NULL); // const char* item_style
    ADD_PROFILING_POINT("elm_naviframe_item_push", "stop");
    elm_naviframe_item_title_enabled_set(naviIt, EINA_FALSE, EINA_FALSE);
    elm_naviframe_item_pop_cb_set(naviIt, naviframeItemPopCallback, NULL);
    setEvasObjectData(Layer::MAIN_LAYOUT, obj);
    return obj;
}

Evas_Object* WindowData::createFocus(Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    // ewkview isn't elementary widget style. This is reason why ewkview focus
    // doesn't restore after focus-out and focus-in. To support focus restore
    // for ewkview, WRT add selectable elementary(button) to manage focus
    Evas_Object* obj = elm_button_add(parent);
    elm_theme_extension_add(NULL, WRT_EDJ_PATH);
    elm_object_style_set(obj, "wrt");
    elm_object_part_content_set(parent, "elm.swallow.content", obj);
    evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_access_object_unregister(obj);
    setEvasObjectData(Layer::FOCUS, obj);
    return obj;
}

Evas_Object* WindowData::createProgressBar(Evas_Object* window, Evas_Object* parent)
{
    AssertMsg(parent != NULL, "Parent is null");
    Evas_Object* obj = evas_object_rectangle_add(evas_object_evas_get(window));
    evas_object_color_set(obj, 91, 166, 255, 255);
    elm_object_part_content_set(parent, "elm.swallow.progress", obj);
    evas_object_resize(obj, 0, 0);
    setEvasObjectData(Layer::PROGRESSBAR, obj);
    return obj;
}

void WindowData::toggleIndicator(bool fullscreen)
{
    LogDebug("fullscreen=" << (fullscreen ? "true" : "false"));

    if (!fullscreen) {
        elm_win_indicator_opacity_set(m_win, ELM_WIN_INDICATOR_OPAQUE);
    } else {
        elm_win_indicator_opacity_set(m_win, ELM_WIN_INDICATOR_TRANSPARENT);
    }
}

void WindowData::winDeleteRequestCallback(void* data,
                                          Evas_Object* /*obj*/,
                                          void* /*eventInfo*/)
{
    DPL_UNUSED_PARAM(data);
    LogDebug("call");
    elm_exit();
}

void WindowData::winProfileChangedCallback(void *data,
                                           Evas_Object* /*obj*/,
                                           void* /*eventInfo*/)
{
    LogDebug("winProfileChangedCallback");
    if (data == NULL) {
        return;
    }
    WindowData* This = static_cast<WindowData *>(data);
    const char* profile = elm_config_profile_get();

    if (DESKTOP_PROFILE == profile) {
        elm_win_indicator_mode_set(This->m_win, ELM_WIN_INDICATOR_HIDE);
        // set desktop icon
        Evas_Object* icon =
            evas_object_image_add(evas_object_evas_get(This->m_win));
        evas_object_image_file_set(icon, DESKTOP_ICON_PATH, NULL);
        elm_win_icon_object_set(This->m_win, icon);
    }
}

Eina_Bool WindowData::naviframeItemPopCallback(void* /*data*/,
                                            Elm_Object_Item* /*it*/)
{
    LogDebug("naviframeItemPopCallback");
    // This return value makes naviframe not to perform a pop operation
    // about this item.
    return EINA_FALSE;
}

void WindowData::miniControlCallback(Ecore_Evas* ee, int domain, int id, void* data, int size)
{
    Assert(ee);
    Assert(data);

    DPL_UNUSED_PARAM(domain);
    DPL_UNUSED_PARAM(id);
    DPL_UNUSED_PARAM(size);

    LogDebug("miniControlCallback");

    WindowData* This = static_cast<WindowData*>(ecore_evas_data_get(ee, USER_DATA_KEY));
    Assert(This);

    char* message = static_cast<char*>(data);
    switch (message[3]) {
        case 'r':
            if (MINICONTROL_MSG_RESUME == message) {
                if (This->m_miniControlCallback->resumeCallback) {
                    This->m_miniControlCallback->resumeCallback();
                }
            } else {
                LogDebug("Unknown msg : " << message);
            }
            break;
        case 'p':
            if (MINICONTROL_MSG_SUSPEND == message) {
                if (This->m_miniControlCallback->suspendCallback) {
                    This->m_miniControlCallback->suspendCallback();
                }
            } else {
                LogDebug("Unknown msg : " << message);
            }
            break;
        default:
            LogDebug("Unknown msg : " << message);

    }
}
