/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    view_logic_security_origin_support_util.cpp
 * @author  Adam Banasiak (a.banasiak@samsung.com)
 * @version 1.0
 * @brief   Support security origin utility
 */

#include "view_logic_security_origin_support_util.h"

#include <Evas.h>
#include <Elementary.h>
#include <efl_assist.h>
#include <dpl/assert.h>
#include <dpl/log/secure_log.h>
#include <dpl/availability.h>
#include <widget_string.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <common/view_logic_get_parent_window_util.h>

namespace ViewModule {

namespace {
const double MAX_POPUP_HEIGHT = 0.80;
const double MAX_SCROLLER_HEIGHT = 0.5;

struct CallbackData {
    Evas_Smart_Cb eaKeyCallback;
};

static void resizeCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo);

static void deleteCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(e);
    DPL_UNUSED_PARAM(eventInfo);

    CallbackData* callbackData = static_cast<CallbackData*>(data);
    Assert(obj);
    if (callbackData) {
        ea_object_event_callback_del(obj, EA_CALLBACK_BACK, callbackData->eaKeyCallback);
        delete callbackData;
    }
    evas_object_event_callback_del(obj, EVAS_CALLBACK_RESIZE, resizeCallback);
}
static void resizeCallback(void* data, Evas* e, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    DPL_UNUSED_PARAM(data);
    DPL_UNUSED_PARAM(e);
    DPL_UNUSED_PARAM(eventInfo);

    Assert(obj);
    Evas_Object* popup = obj;
    int popupH;
    evas_object_geometry_get(popup, 0, 0, 0, &popupH);

    Evas_Object* parent = PopupUtil::getParentWindow(popup);
    int parentW, parentH;
    evas_object_geometry_get(parent, 0, 0, &parentW, &parentH);

    // compare current popup height with screen height.
    // To avoid popup is filled full screen, used magic number to be filled 80% of screen height.
    // TODO: Automatically add scroller feature should implement in the elementary
    double threshold = parentH * MAX_POPUP_HEIGHT;
    double currentH = popupH;
    if (threshold < currentH) {
        _D("text is overflow popup height. add scroller");
        Evas_Object* layout = elm_object_content_get(obj);
        Evas_Object* label = elm_object_part_content_get(layout, "elm.swallow.label");

        Evas_Object* scroller = elm_scroller_add(layout);
        elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_AUTO);
        evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_scroller_content_min_limit(scroller, EINA_TRUE, EINA_TRUE);
        int scrollerHeight = parentW > parentH ? parentH : parentW;
        evas_object_size_hint_max_set(scroller, -1, scrollerHeight * MAX_SCROLLER_HEIGHT);

        elm_object_part_content_unset(layout, "elm.swallow.label");
        elm_object_content_set(scroller, label);
        elm_object_part_content_set(layout, "elm.swallow.label", scroller);
        evas_object_show(layout);
    }
}
} // anonymous namespace

Evas_Object* SecurityOriginSupportUtil::createPopup(
    Evas_Object* window,
    const char* bodyText,
    const char* checkText,
    Evas_Smart_Cb buttonCallback,
    Evas_Smart_Cb keyCallback,
    void* data)
{
    _D("createPopup");
    Evas_Object* parentWindow = PopupUtil::getParentWindow(window);
    Evas_Object* popup = elm_popup_add(parentWindow);

    CallbackData* callbackData = NULL;
    if (keyCallback) {
        callbackData = new CallbackData;
        callbackData->eaKeyCallback = keyCallback;
        ea_object_event_callback_add(popup, EA_CALLBACK_BACK, keyCallback, data);
    }
    evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, deleteCallback, static_cast<void*>(callbackData));
    evas_object_event_callback_add(popup, EVAS_CALLBACK_RESIZE, resizeCallback, NULL);

   // elm_object_style_set(popup, "popup/default");
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object* layout = elm_layout_add(popup);
    elm_layout_file_set(layout, WRT_EDJ_PATH, "popupWithCheck");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object* label = elm_label_add(popup);
    elm_label_line_wrap_set(label, ELM_WRAP_WORD);
    elm_object_text_set(label, bodyText);
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Evas_Object* check = elm_check_add(layout);
    elm_object_style_set(check, "multiline");
    elm_object_text_set(check, checkText);

    elm_object_part_content_set(layout, "elm.swallow.label", label);
    elm_object_part_content_set(layout, "elm.swallow.checkbox", check);
    elm_object_content_set(popup, layout);

    Evas_Object* btn1 = elm_button_add(popup);
    elm_object_style_set(btn1, "popup");
    elm_object_text_set(btn1, WRT_OPT_DENY);
    elm_object_part_content_set(popup, "button1", btn1);
    evas_object_smart_callback_add(btn1, "clicked", buttonCallback, data);

    Evas_Object* btn2 = elm_button_add(popup);
    elm_object_style_set(btn2, "popup");
    elm_object_text_set(btn2, WRT_OPT_ALLOW);
    elm_object_part_content_set(popup, "button2", btn2);
    evas_object_smart_callback_add(btn2, "clicked", buttonCallback, data);

    return popup;
}

Evas_Object* SecurityOriginSupportUtil::getPopup(Evas_Object* button)
{
    Assert(button);

    Evas_Object* popup = button;
    while (strcmp(elm_object_widget_type_get(popup), "elm_popup")) {
        popup = elm_object_parent_widget_get(popup);
        if (!popup) {
            return NULL;
        }
    }
    return popup;
}

Evas_Object* SecurityOriginSupportUtil::getCheck(Evas_Object* popup)
{
    Assert(popup);
    if (strcmp(elm_object_widget_type_get(popup), "elm_popup")) {
        return NULL;
    }
    Evas_Object* check = elm_object_part_content_get(
            elm_object_content_get(popup),
            "elm.swallow.checkbox");
    return check;
}

SecurityOriginDB::Result SecurityOriginSupportUtil::getResult(
    Evas_Object* button)
{
    using namespace SecurityOriginDB;

    Assert(button);
    // get popup evas_object
    Evas_Object* popup = getPopup(button);
    if (popup == NULL) {
        return RESULT_UNKNOWN;
    }
    bool allow = !strcmp(WRT_OPT_ALLOW, elm_object_text_get(button));

    // get check evas_object
    Evas_Object* check = getCheck(popup);
    if (check == NULL) {
        return RESULT_UNKNOWN;
    }
    if (allow) {
        return elm_check_state_get(check) ? RESULT_ALLOW_ALWAYS :
               RESULT_ALLOW_ONCE;
    } else {
        return elm_check_state_get(check) ? RESULT_DENY_ALWAYS :
               RESULT_DENY_ONCE;
    }
}

bool SecurityOriginSupportUtil::isNeedHelpPopup(Evas_Object* popup)
{
    Assert(popup);

    if (strcmp(elm_object_widget_type_get(popup), "elm_popup")) {
        return false;
    }
    Evas_Object* check = getCheck(popup);
    if (check && elm_check_state_get(check)) {
        return true;
    }
    return false;
}

} // namespace ViewModule
