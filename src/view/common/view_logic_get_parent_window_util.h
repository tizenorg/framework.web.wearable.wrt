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
 * @file    view_logic_get_parent_window_util.h
 * @author  Tae-Jeong Lee (taejeong.lee@samsung.com)
 */

#ifndef VIEW_LOGIC_GET_PARENT_WINDOW_UTIL_H_
#define VIEW_LOGIC_GET_PARENT_WINDOW_UTIL_H_

#include <Elementary.h>
#include <dpl/log/log.h>

namespace ViewModule {
namespace PopupUtil {

static Evas_Object* getParentWindow(Evas_Object* object)
{
    Evas_Object* parent = elm_object_parent_widget_get(object);
    Evas_Object* win = parent;

    while (parent) {
        const char* type = elm_object_widget_type_get(parent);
        if (type) {
            if (!strncmp(type, "elm_win", strlen("elm_win"))) {
                win = parent;
                break;
            }
        }
        parent = elm_object_parent_widget_get(parent);
    }

    if (!win)
    {
        LogError("Parent window was not found!");
        win = object;
    }

    return win;
}

} //namespace ViewModule
} //namespace PopupUtil
#endif //VIEW_LOGIC_GET_PARENT_WINDOW_UTIL_H_
