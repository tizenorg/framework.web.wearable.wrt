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
 * @file        window_data.h
 * @author      Jaroslaw Osmanski (j.osmanski@samsung.com)
 * @version     1.0
 * @brief       Window data header file.
 */

#ifndef WINDOW_INITIALIZE_H_
#define WINDOW_INITIALIZE_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <dpl/framework_efl.h>
#include <dpl/noncopyable.h>

enum class Layer {
    WINDOW      = 0,
    BG          = 1,
    CONFORMANT  = 2,
    TOP_LAYOUT  = 3,
    NAVIFRAME   = 4,
    MAIN_LAYOUT = 5,
    FOCUS       = 6,
    PROGRESSBAR = 7
};

enum AppCategory {
    APP_CATEGORY_NORMAL = 0,
    APP_CATEGORY_IDLE_CLOCK
};

class WindowData : private DPL::Noncopyable
{
  public:
    explicit WindowData(AppCategory category, unsigned long pid, bool manualInit = false);
    virtual ~WindowData();

    void init();
    void postInit();
    bool initScreenReaderSupport(bool isSupportAccessibility);
    void initTheme(void);

    Evas_Object* getEvasObject(Layer layer);
    void setWebview(Evas_Object* evas_object);
    void unsetWebview();

    void smartCallbackAdd(
        Layer layer,
        const char* event,
        Evas_Smart_Cb callback,
        const void* data);
    void smartCallbackDel(
        Layer layer,
        const char* event,
        Evas_Smart_Cb callback);
    void signalEmit(Layer layer, const char* emission, const char* source);

    void setViewMode(bool fullscreen, bool backbutton);
    void setOrientation(int angle);
    void toggleFullscreen(bool fullscreen);
    void toggleTransparent(bool transparent);
    void updateProgress(double value);

  private:
    Evas_Object* m_win;
    Evas_Object* m_bg;
    Evas_Object* m_conformant;
    Evas_Object* m_topLayout;
    Evas_Object* m_naviframe;
    Evas_Object* m_mainLayout;
    Evas_Object* m_focus;
    Evas_Object* m_progressbar;
    bool m_initialized;
    bool m_currentViewModeFullScreen;
    AppCategory m_category;

    typedef std::map<Layer, Evas_Object*> EvasObjectData;
    typedef std::map<Layer, Evas_Object*>::iterator EvasObjectDataIt;
    EvasObjectData m_evasObjectData;
    void setEvasObjectData(Layer layer, Evas_Object* obj);

    Evas_Object* createWindow(unsigned long pid);
    Evas_Object* createBg(Evas_Object* parent);
    Evas_Object* createConformant(Evas_Object* parent);
    Evas_Object* createTopLayout(Evas_Object* parent);
    Evas_Object* createNaviframe(Evas_Object* parent);
    Evas_Object* createMainLayout(Evas_Object* parent);
    Evas_Object* createFocus(Evas_Object* parent);
    Evas_Object* createProgressBar(Evas_Object* window, Evas_Object* parent);

    void toggleIndicator(bool fullscreen);
    static void winDeleteRequestCallback(void* data,
                                         Evas_Object* obj,
                                         void* eventInfo);
    static void winProfileChangedCallback(void* data,
                                          Evas_Object* obj,
                                          void* eventInfo);
    static Eina_Bool naviframeItemPopCallback(void *data,
                                          Elm_Object_Item *it);
 };

typedef std::shared_ptr<WindowData> WindowDataPtr;

#endif /* WINDOW_INITIALIZE_H_ */
