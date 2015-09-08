/*
  * Copyright 2013  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.1 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://floralicense.org/license/
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
/**
 * @file       client_orientation_support.cpp
 * @author     Grzegorz Rynkowski (g.rynkowski@samsung.com)
 * @brief      source file to support window orientation of web application
 */

#include "client_orientation_support.h"

#include <dpl/assert.h>
#include <dpl/availability.h>
#include <dpl/foreach.h>
#include <dpl/log/secure_log.h>
#include <Elementary.h>
#include <Evas.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <widget_data_types.h>

namespace ClientModule {

namespace {
const char* const WM_ROTATION_CHANGED_CALLBACK =  "wm,rotation,changed";
const int ANGLE_PORTRAIT = 0;
const int ANGLE_LANDSCAPE = 270;
}

OrientationSupport::OrientationSupport(const WindowDataPtr& windowData,
                                     const WRT::RunnableWidgetObjectPtr& widget)
        : m_windowData(windowData),
          m_widget(widget),
          m_rotationAngle(0)
{
}

void OrientationSupport::setWindowOrientation(int angle)
{
    Assert(m_windowData);
    m_windowData->setOrientation(angle);
}

void OrientationSupport::setInitialWindowOrientation(
                                     WidgetSettingScreenLock rotationValue)
{
    switch(rotationValue)  {
    case Screen_AutoRotation:
        break;
    case Screen_Landscape:
        setWindowOrientation(OrientationAngle::Window::Landscape::PRIMARY);
        break;
    case Screen_Portrait:
    default:
        setWindowOrientation(OrientationAngle::Window::Portrait::PRIMARY);
        break;
    }
}

bool OrientationSupport::setAutoRotation()
{
    Assert(m_windowData);
    Evas_Object* window = m_windowData->getEvasObject(Layer::WINDOW);

    if (elm_win_wm_rotation_supported_get(window)) {
        const int rots[4] = {OrientationAngle::Window::Portrait::PRIMARY,
                             OrientationAngle::Window::Portrait::SECONDARY,
                             OrientationAngle::Window::Landscape::PRIMARY,
                             OrientationAngle::Window::Landscape::SECONDARY};
        elm_win_wm_rotation_available_rotations_set(window, rots, 4);
        registerRotationCallback(&OrientationSupport::autoRotationCallback, this);
#if USE(WEBKIT_MANUAL_ROTATION)
        m_rotationAngle = elm_win_rotation_get(window);
#endif
        return true;
    }
    return false;
}

#if USE(WEBKIT_MANUAL_ROTATION)
void OrientationSupport::setRotationDone(void)
{
    _D("called");

    Assert(m_windowData);
    Evas_Object* win = m_windowData->getEvasObject(Layer::WINDOW);
    Assert(win);
    if (EINA_TRUE == elm_win_wm_rotation_manual_rotation_done_get(win)) {
        elm_win_wm_rotation_manual_rotation_done(win);
    }
}

void OrientationSupport::setManualRotation(bool enable)
{
    _D("called");

    Assert(m_windowData);
    Evas_Object* win = m_windowData->getEvasObject(Layer::WINDOW);

    Assert(win);
    elm_win_wm_rotation_manual_rotation_done_set(win, enable ? EINA_TRUE : EINA_FALSE);
}
#endif

bool OrientationSupport::isOrientationPrepared(WidgetSettingScreenLock rotationValue)
{
    _D("called");

    Assert(m_windowData);
    Evas_Object* win = m_windowData->getEvasObject(Layer::WINDOW);
    Assert(win);
    int currentAngle = elm_win_rotation_get(win);
    if (currentAngle == -1) {
        _E("elm_win_rotation_get failed");
        Assert(false);
    }

    switch(rotationValue) {
        case Screen_Landscape:
            return currentAngle == ANGLE_LANDSCAPE;
        case Screen_Portrait:
            return currentAngle == ANGLE_PORTRAIT;
        case Screen_AutoRotation:
        default:
            return true;
    }
}

void OrientationSupport::registerRotationCallback(const Evas_Smart_Cb callback,
                                                  const void *data)
{
    Assert(m_windowData);
    Evas_Object* window = m_windowData->getEvasObject(Layer::WINDOW);
    _D("add rotation callback: %s (%p)", WM_ROTATION_CHANGED_CALLBACK, callback);
    evas_object_smart_callback_add(window,
                                   WM_ROTATION_CHANGED_CALLBACK,
                                   callback,
                                   data);
    m_registeredCallbacks.push_back(
            Callback(WM_ROTATION_CHANGED_CALLBACK, callback));
}

void OrientationSupport::unregisterRotationCallback(const Evas_Smart_Cb callback)
{
    FOREACH(it, m_registeredCallbacks)
    {
        if (it->second == callback) {
            Assert(m_windowData);
            Evas_Object* win = m_windowData->getEvasObject(Layer::WINDOW);
            Assert(win);
            _D("remove rotation callback: %s (%p)", WM_ROTATION_CHANGED_CALLBACK, callback);
            evas_object_smart_callback_del(win, it->first.c_str(), it->second);
            m_registeredCallbacks.erase(it);
            return;
        }
    }
}

void OrientationSupport::unregisterRotationCallbacks()
{
    Assert(m_windowData);
    Evas_Object* wkView = m_windowData->getEvasObject(Layer::WINDOW);
    FOREACH(it, m_registeredCallbacks)
    {
        _D("remove rotation callback: %s (%p)", it->first.c_str(), it->second);
        evas_object_smart_callback_del(wkView, it->first.c_str(), it->second);
    }
}

void OrientationSupport::setEwkInitialOrientation(
                                     WidgetSettingScreenLock rotationValue)
{
    Assert(m_widget);

    Evas_Object* obj =  m_widget->GetCurrentWebview();
    switch(rotationValue)  {
      case Screen_Landscape:
          ewk_view_orientation_send(obj,
                                    OrientationAngle::W3C::Landscape::PRIMARY);
          break;
      case Screen_Portrait:
      case Screen_AutoRotation:
      default:
          ewk_view_orientation_send(obj,
                                    OrientationAngle::W3C::Portrait::PRIMARY);
          break;
      }
}

void OrientationSupport::rotatePrepared(Evas_Object* obj)
{
    _D("rotatePrepared");
    Assert(m_windowData);
    int winAngle = elm_win_rotation_get(
            m_windowData->getEvasObject(Layer::WINDOW));
    int w3cAngle;

    switch(winAngle)  {
    case OrientationAngle::Window::Portrait::PRIMARY:
        w3cAngle = OrientationAngle::W3C::Portrait::PRIMARY;
        break;
    case OrientationAngle::Window::Portrait::SECONDARY:
        w3cAngle = OrientationAngle::W3C::Portrait::SECONDARY;
        break;
    case OrientationAngle::Window::Landscape::PRIMARY:
        w3cAngle = OrientationAngle::W3C::Landscape::PRIMARY;
        break;
    case OrientationAngle::Window::Landscape::SECONDARY:
        w3cAngle = OrientationAngle::W3C::Landscape::SECONDARY;
        break;
    default:
        _W("unknown angle");
        return;
    }
    ewk_view_orientation_send(obj, w3cAngle);
}

void OrientationSupport::updateRotationAngle(void)
{
    Evas_Object* win = m_windowData->getEvasObject(Layer::WINDOW);
    int newAngle = elm_win_rotation_get(win);
    if (newAngle == -1) {
        _W("elm_win_rotation_get failed");
        return;
    }

#if USE(WEBKIT_MANUAL_ROTATION)
    if (EINA_FALSE == elm_win_wm_rotation_manual_rotation_done_get(win)) {
        _D("Rotate by platform");
        m_rotationAngle = newAngle;
        return;
    }

    if ((m_rotationAngle + newAngle) % 180 == 0)
    {
        _D("Rotate by wrt");
        // In case of rotate 180 degress, setRotationDone by WRT instead of webkit.
        //    Webkit doesn't call "rotate,prepared" callback.
        setRotationDone();
    }
    m_rotationAngle = newAngle;
#endif
}

//static
void OrientationSupport::autoRotationCallback(void* data,
                                     Evas_Object* obj,
                                     void* event)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(event);
    _D("entered");
    OrientationSupport* This = static_cast<OrientationSupport*>(data);
    Assert(This->m_widget);
    This->rotatePrepared(This->m_widget->GetCurrentWebview());
    This->updateRotationAngle();
}

void OrientationSupport::resetOrientation()
{
    Assert(m_widget);
    rotatePrepared(m_widget->GetCurrentWebview());
    updateRotationAngle();
}

} // namespace ClientModule
