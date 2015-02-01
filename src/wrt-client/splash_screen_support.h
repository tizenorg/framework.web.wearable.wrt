/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        splash_screen_support.h
 * @author      Andrzej Surdej (a.surdej@samsung.com)
 * @brief       Header file for supporting splash screen handling
 */

#ifndef WRT_SPLASH_SCREEN_SUPPORT_H
#define WRT_SPLASH_SCREEN_SUPPORT_H

#include <Evas.h>
#include <Elementary.h>

class SplashScreenSupport
{
  public:
    SplashScreenSupport(Evas_Object* parent, const char* image_path, bool indicator = true, bool landscape = false);
    ~SplashScreenSupport();

    void startSplashScreen();
    void stopSplashScreen();
    void stopSplashScreenBuffered();
    bool isShowing();
    void resizeSplashScreen();
    static Eina_Bool timerCallback(void *data);

  private:
    Evas_Object* m_parent;
    Evas_Object* m_splashScreen;
    Evas_Object* m_splashScreenBg;
    Ecore_Timer* m_timer;

    int m_deviceWidth;
    int m_deviceHeight;
    int m_imageWidth;
    int m_imageHeight;

    bool m_initialized;
    bool m_isShowing;
    bool m_indicator;
};

#endif /* WRT_SPLASH_SCREEN_SUPPORT_H */
