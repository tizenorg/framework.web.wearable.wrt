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
 * @file        MockViewModule.h
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       Mock for view module
 */

#ifndef MOCKVIEWMODULE_H
#define MOCKVIEWMODULE_H

#include<i_view_module.h>

class MockViewModule : public ViewModule::IViewModule
{
public:
    MockViewModule();
    bool createView(Ewk_Context* context, Evas_Object* window);
    void prepareView(WidgetModel *, const std::string &);
    void showWidget();
    void hideWidget();
    void suspendWidget();
    void resumeWidget();
    void resetWidgetFromSuspend();
    void resetWidgetFromResume();
    void backward();
    void reloadStartPage(bool isbackground);
    Evas_Object* getCurrentWebview();
    void fireJavascriptEvent(int event, void* data);
    void setUserCallbacks(const WRT::UserDelegatesPtr& cbs);
    void checkSyncMessageFromBundle(
            const char* name,
            const char* body,
            char** returnData);
    void checkAsyncMessageFromBundle(
            const char* name,
            const char* body);
    void downloadData(const char* url);
    void activateVibration(bool on, uint64_t time);
};

#endif // MOCKVIEWMODULE_H
