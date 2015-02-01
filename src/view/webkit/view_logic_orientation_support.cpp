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
 * @file       view_logic_orientation_support.cpp
 * @author     Jihoon Chung (jihoon.chung@samsung.com)
 * @version    1.0
 */

 #include "view_logic_orientation_support.h"

#include <dpl/log/log.h>
#include <widget_data_types.h>

#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace ViewModule {

int OrientationSupport::getWinOrientationAngle(int ewkOrientation)
{
    int angle;
    if (ewkOrientation & EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY) {
        angle = OrientationAngle::Window::Portrait::PRIMARY;
    } else if (ewkOrientation & EWK_SCREEN_ORIENTATION_LANDSCAPE_PRIMARY) {
        angle = OrientationAngle::Window::Landscape::PRIMARY;
    } else if (ewkOrientation & EWK_SCREEN_ORIENTATION_PORTRAIT_SECONDARY) {
        angle = OrientationAngle::Window::Portrait::SECONDARY;
    } else if (ewkOrientation & EWK_SCREEN_ORIENTATION_LANDSCAPE_SECONDARY) {
        angle = OrientationAngle::Window::Landscape::SECONDARY;
    } else {
        LogDebug("Wrong orientation value is passed");
        Assert(false);
    }
    return angle;
}

int OrientationSupport::getW3COrientationAngle(int ewkOrientation)
{
    int angle;
    if (ewkOrientation & EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY) {
        angle = OrientationAngle::W3C::Portrait::PRIMARY;
    } else if (ewkOrientation & EWK_SCREEN_ORIENTATION_LANDSCAPE_PRIMARY) {
        angle = OrientationAngle::W3C::Landscape::PRIMARY;
    } else if (ewkOrientation & EWK_SCREEN_ORIENTATION_PORTRAIT_SECONDARY) {
        angle = OrientationAngle::W3C::Portrait::SECONDARY;
    } else if (ewkOrientation & EWK_SCREEN_ORIENTATION_LANDSCAPE_SECONDARY) {
        angle = OrientationAngle::W3C::Landscape::SECONDARY;
    } else {
        LogDebug("Wrong orientation value is passed");
        Assert(false);
    }
    return angle;
}

void OrientationSupport::setEwkOrientation(Evas_Object* ewk,
                                           int angle)
{
    LogDebug("setOrientation called");
    ewk_view_orientation_send(ewk, angle);
}
} // ViewModule
