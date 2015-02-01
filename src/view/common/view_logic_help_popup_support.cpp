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
 * @file    view_logic_help_popup_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 */

#include "view_logic_help_popup_support.h"

#include <efl_assist.h>
#include <Elementary.h>
#include <dpl/assert.h>
#include <dpl/availability.h>
#include <dpl/log/secure_log.h>

#include <common/view_logic_get_parent_window_util.h>
#include <widget_string.h>

namespace ViewModule {
namespace {
Evas_Object* getPopup(Evas_Object* object);
static void buttonClickedCallback(void* data, Evas_Object* obj, void* eventInfo);

Evas_Object* getPopup(Evas_Object* object)
{
    Assert(object);

    Evas_Object* popup = object;
    while (strcmp(elm_object_widget_type_get(popup), "elm_popup")) {
        popup = elm_object_parent_widget_get(popup);
        if (!popup) {
            return NULL;
        }
    }
    return popup;
}

static void buttonClickedCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(obj);

    DPL_UNUSED_PARAM(data);
    DPL_UNUSED_PARAM(eventInfo);

    Evas_Object* popup = getPopup(obj);
    if (!popup) {
        _W("Fail to get popup object");
        return;
    }
    evas_object_hide(popup);
    evas_object_del(popup);
}
} // namespace anonymous

void HelpPopupSupport::showClearDefaultPopup(Evas_Object* object)
{
    _D("called");

    Assert(object);

    // create popup
    Evas_Object* parentWindow = PopupUtil::getParentWindow(object);
    Evas_Object* popup = elm_popup_add(parentWindow);
    elm_object_text_set(popup, WRT_POP_CLEAR_DEFAULT_SETTINGS);
    ea_object_event_callback_add(popup, EA_CALLBACK_BACK, ea_popup_back_cb, NULL);

    // create button
    Evas_Object* button = elm_button_add(popup);
    elm_object_style_set(button, "popup");
    elm_object_text_set(button, WRT_SK_OK);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", buttonClickedCallback, NULL);

	evas_object_show(popup);
}
} // namespace ViewModule
