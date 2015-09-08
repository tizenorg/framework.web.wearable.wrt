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
 * @file    core_module.cpp
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @version 1.0
 * @brief   File contains defitinions of RunnableWidgetObject implementation.
 */

#ifndef RUNNABLE_WIDGET_OBJECT_H_
#define RUNNABLE_WIDGET_OBJECT_H_

//forward declaration
namespace WRT {
namespace State {
class RunnableWidgetObjectState;
class StateChange;
}
}

#include "i_runnable_widget_object.h"

#include <string>
#include <memory>

#include <widget_model.h>
#include <dpl/exception.h>
#include <i_view_module.h>
#include <i_context_manager.h>

namespace WRT {
class RunnableWidgetObject : public IRunnableWidgetObject
{
public:
    RunnableWidgetObject(WidgetModelPtr &model);
    virtual ~RunnableWidgetObject();

    bool CheckBeforeLaunch();
    bool PrepareView(const std::string &startUrl,
            Evas_Object *window, Ewk_Context* ewkContext = NULL, int category = 0);
    void Show(); //asynchronous function
    void Hide();
    void Suspend();
    void Resume();
    void Reset();
    void TimeTick(long time);
    void AmbientTick(long time);
    void AmbientModeChanged(bool ambient_mode);
    void ReloadStartPage();
    Evas_Object* GetCurrentWebview();
    void SetUserDelegates(const UserDelegatesPtr& cbs);
    void Backward();
    void FireJavascriptEvent(int event, void* data);

    void setViewModule(ViewModule::IViewModulePtr ptr);
    void setContextManagerFactoryMethod(ViewModule::ContextManagerFactoryMethod method);
  private:

    bool CheckWACTestCertififedWidget();
    void setNewState(std::shared_ptr<WRT::State::RunnableWidgetObjectState> sptr);

    WidgetModelPtr m_widgetModel;
    ViewModule::IViewModulePtr m_view;
    std::shared_ptr<State::RunnableWidgetObjectState> m_guardstate;
    ViewModule::ContextManagerPtr m_ewkContextManager;

    //factor method to be used for creation of context manager when needed
    ViewModule::ContextManagerFactoryMethod m_contextManagerFactoryMethod;

    friend class State::StateChange;
};
} /* namespace WRT */
#endif /* RUNNABLE_WIDGET_OBJECT_H_ */
