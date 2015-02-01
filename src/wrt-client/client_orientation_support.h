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
 * @file       client_orientation_support.h
 * @author     Grzegorz Rynkowski (g.rynkowski@samsung.com)
 * @brief      header file to support window orientation of web application
 */

#ifndef CLIENT_ORIENTATION_SUPPORT_H_
#define CLIENT_ORIENTATION_SUPPORT_H_

#include <string>
#include <vector>
#include <utility>

#include <dpl/platform.h>
#include <Evas.h>
#include <i_runnable_widget_object.h>
#include <widget_data_types.h>

#include "window_data.h"

namespace ClientModule {

class OrientationSupport {
  public:
    OrientationSupport(const WindowDataPtr& windowData,
                       const WRT::RunnableWidgetObjectPtr& widget);

    void setInitialWindowOrientation(WidgetSettingScreenLock rotationValue);
    void setEwkInitialOrientation(WidgetSettingScreenLock rotationValue);
    bool setAutoRotation();
#if USE(WEBKIT_MANUAL_ROTATION)
    void setRotationDone(void);
    void setManualRotation(bool enable);
#endif
    bool isOrientationPrepared(WidgetSettingScreenLock rotationValue);

    void registerRotationCallback(const Evas_Smart_Cb callback, const void *data);
    void unregisterRotationCallback(const Evas_Smart_Cb callback);
    void unregisterRotationCallbacks();
    void resetOrientation();

  private:
    void setWindowOrientation(int angle);
    void rotatePrepared(Evas_Object* obj);
    void updateRotationAngle(void);
    static void autoRotationCallback(void* data, Evas_Object* obj, void* event);

    WindowDataPtr m_windowData;
    WRT::RunnableWidgetObjectPtr m_widget;
    // TODO: separate by base class(OrientationManualRotationSupport)
    int m_rotationAngle;

    typedef std::pair<std::string, Evas_Smart_Cb> Callback;
    typedef std::vector<Callback> CallbacksList;
    CallbacksList m_registeredCallbacks;
};

} // namespace ClientModule
#endif // CLIENT_ORIENTATION_SUPPORT_H_
