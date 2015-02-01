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
 * @file        splash_screen_support.cpp
 * @author      Andrzej Surdej (a.surdej@samsung.com)
 * @brief       Implementation file for splash screen support
 */

#include "splash_screen_support.h"
#include <dpl/log/log.h>

const short  SPLASH_SCREEN_LAYER = 10;
const double SPLASH_SCREEN_MAXIUM_TIMER = 5.0;   // sec
const double SPLASH_SCREEN_BUFFER_TIMER = 0.5;  //  sec
const int    INDICATOR_H = 60;

static void evas_object_reposition(Evas_Object* obj, int x, int y, int w, int h)
{
    evas_object_resize(obj,w, h);
    evas_object_move(obj, x, y);
}

SplashScreenSupport::SplashScreenSupport(Evas_Object* parent, const char* image_path, bool indicator, bool landscape) :
    m_parent(parent),
    m_splashScreen(NULL),
    m_splashScreenBg(NULL),
    m_timer(NULL),
    m_deviceWidth(0),
    m_deviceHeight(0),
    m_imageWidth(0),
    m_imageHeight(0),
    m_initialized(false),
    m_isShowing(false),
    m_indicator(indicator)
{
    LogDebug("enter");

    if (!m_parent || !image_path)
    {
        LogError("Invalid parameter");
        return;
    }

    // create evas object
    m_splashScreenBg = evas_object_rectangle_add(evas_object_evas_get(m_parent));
    evas_object_color_set(m_splashScreenBg, 255, 255, 255, 255);

    m_splashScreen = elm_image_add(m_splashScreenBg);
    elm_image_no_scale_set(m_splashScreen, FALSE);

    if (elm_image_file_set(m_splashScreen, image_path, NULL))
    {
        // check animation
        if (elm_image_animated_available_get(m_splashScreen))
        {
            elm_image_animated_set(m_splashScreen, EINA_TRUE);
            elm_image_animated_play_set(m_splashScreen, EINA_FALSE);
        }

        // get contents area
        Evas_Coord x, y, w, h;
        evas_object_geometry_get(m_parent, &x, &y, &w, &h);

        m_deviceWidth  = (w < h) ? w : h;
        m_deviceHeight = (w > h) ? w : h;

        if (!landscape)
        {
            x = 0;
            y = 0;
            w = m_deviceWidth;
            h = m_deviceHeight;

// there are no indicator space in wearable device
#if 0
            if (m_indicator)
            {
                int indicator_h = INDICATOR_H * elm_config_scale_get();

                y += indicator_h;
                h -= indicator_h;
            }
#endif
        }
        else
        {
            x = 0;
            y = 0;
            w = m_deviceHeight;
            h = m_deviceWidth;
        }

        // fit to width
        elm_image_object_size_get(m_splashScreen, &m_imageWidth, &m_imageHeight);

        if (m_imageWidth == 0 || m_imageHeight == 0)
        {
            LogError("Splash screen image size error!");

            m_imageWidth = w;
            m_imageHeight = h;
        }

        double ratio_win    = (double)w/h;
        double ratio_image  = (double)m_imageWidth/m_imageHeight;

        // set evas position
        evas_object_reposition(m_splashScreenBg, x, y, w, h);
        evas_object_reposition(m_splashScreen,   x, y, w, h);

        double scaled_image_w = w;
        double scaled_image_h = (w/ratio_image);

        Evas_Object* imageObject = elm_image_object_get(m_splashScreen);

        if (ratio_image <= ratio_win)
        {
            evas_object_image_fill_set(imageObject, 0, 0-((scaled_image_h-h)/2), scaled_image_w, scaled_image_h);
            evas_object_reposition(imageObject, x, y, w, h);
        }
        else
        {
            evas_object_image_fill_set(imageObject, 0, 0, scaled_image_w, scaled_image_h);
            evas_object_reposition(imageObject, 0, y+(h-scaled_image_h)/2, scaled_image_w, scaled_image_h);
        }

        m_initialized = true;
    }
}

SplashScreenSupport::~SplashScreenSupport()
{
    LogDebug("enter");

    if (m_initialized)
    {
        evas_object_del(m_splashScreen);
        evas_object_del(m_splashScreenBg);
    }

    if (m_timer)
    {
        ecore_timer_del(m_timer);
        m_timer = NULL;
    }
}

Eina_Bool SplashScreenSupport::timerCallback(void *data)
{
    LogDebug("enter");

    SplashScreenSupport* This = static_cast<SplashScreenSupport*>(data);

    if (This->isShowing())
    {
        This->stopSplashScreen();
    }

    return ECORE_CALLBACK_CANCEL;
}

void SplashScreenSupport::startSplashScreen()
{
    LogDebug("splashImageOn");

    if (m_initialized)
    {
        if (elm_image_animated_get(m_splashScreen) == EINA_TRUE)
        {
            elm_image_animated_play_set(m_splashScreen, EINA_TRUE);
        }

        evas_object_layer_set(m_splashScreenBg, SPLASH_SCREEN_LAYER);
        evas_object_layer_set(m_splashScreen,   SPLASH_SCREEN_LAYER);

        evas_object_show(m_splashScreenBg);
        evas_object_show(m_splashScreen);

        m_isShowing = true;

        if (m_timer)
        {
            ecore_timer_del(m_timer);
        }

        m_timer = ecore_timer_add(SPLASH_SCREEN_MAXIUM_TIMER, timerCallback, this);
    }
}


void SplashScreenSupport::stopSplashScreenBuffered()
{
    if (m_isShowing)
    {
        if (m_timer)
        {
            ecore_timer_del(m_timer);
        }

        m_timer = ecore_timer_add(SPLASH_SCREEN_BUFFER_TIMER, timerCallback, this);
    }
}

void SplashScreenSupport::stopSplashScreen()
{
    LogDebug("splashImageOff");

    if (m_isShowing)
    {
        if (elm_image_animated_get(m_splashScreen) == EINA_TRUE)
        {
            elm_image_animated_play_set(m_splashScreen, EINA_FALSE);
        }

        evas_object_hide(m_splashScreen);
        evas_object_hide(m_splashScreenBg);

        evas_object_del(m_splashScreen);
        evas_object_del(m_splashScreenBg);

        m_initialized = false;
        m_isShowing = false;

        if (m_timer)
        {
            ecore_timer_del(m_timer);
            m_timer = NULL;
        }
    }
}

bool SplashScreenSupport::isShowing()
{
    return m_isShowing;
}

void SplashScreenSupport::resizeSplashScreen()
{
    if (m_initialized && m_isShowing)
    {
        int x, y, w, h;
        int angle = elm_win_rotation_get(m_parent);

        if (angle == 0 || angle == 180)
        {
            x = 0;
            y = 0;
            w = m_deviceWidth;
            h = m_deviceHeight;
// there are no indicator space in wearable device
#if 0
            if (m_indicator)
            {
                int indicator_h = INDICATOR_H * elm_config_scale_get();

                y += indicator_h;
                h -= indicator_h;
            }
#endif
        }
        else
        {
            x = 0;
            y = 0;
            w = m_deviceHeight;
            h = m_deviceWidth;
        }

        // fit to width
        double ratio_win    = (double)w/h;
        double ratio_image  = (double)m_imageWidth/m_imageHeight;

        evas_object_reposition(m_splashScreenBg, x, y, w, h);
        evas_object_reposition(m_splashScreen,   x, y, w, h);

        double scaled_image_w = w;
        double scaled_image_h = (w/ratio_image);

        Evas_Object* imageObject = elm_image_object_get(m_splashScreen);

        if (ratio_image <= ratio_win)
        {
            evas_object_image_fill_set(imageObject, 0, 0-((scaled_image_h-h)/2), scaled_image_w, scaled_image_h);
            evas_object_reposition(imageObject, x, y, w, h);
        }
        else
        {
            evas_object_image_fill_set(imageObject, 0, 0, scaled_image_w, scaled_image_h);
            evas_object_reposition(imageObject, 0, y+(h-scaled_image_h)/2, scaled_image_w, scaled_image_h);
        }

    }
}
