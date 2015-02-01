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
 * @file    runnable_widget_object.cpp
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @version 1.0
 * @brief   File contains defitinions of RunnableWidgetObject implementation.
 */

#include <runnable_widget_object.h>
#include <privilege-control.h>
#include <dpl/exception.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/utils/wrt_global_settings.h>
#include <appcore-common.h>
#include <profiling_util.h>
#include <signal.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <runnable_widget_object_state.h>

namespace { //Anonymous
const unsigned int UID_ROOT = 0;
const char* const MESSAGE_NAME_INITIALIZE = "ToInjectedBundle::INIT";
const char* const MESSAGE_NAME_SHUTDOWN = "ToInjectedBundle::SHUTDOWN";
} // namespace anonymous

namespace WRT {
RunnableWidgetObject::RunnableWidgetObject(WidgetModelPtr &model) :
        m_widgetModel(model),
        m_view(ViewModule::createView()),
        m_contextManagerFactoryMethod(ViewModule::makeContextManagerFactoryMethod())
{
    //set initial state of runnable object
    m_guardstate = std::shared_ptr<State::RunnableWidgetObjectState>(
            new State::InitialState(*this));
    // If current uid is 'root', change privilege to apps
    if (UID_ROOT == getuid()) {
        // Set privilege by tizen id
        // this code prevent that widget launch with "root" permission,
        // when developers launch by command in the shell

        perm_app_set_privilege(
                DPL::ToUTF8String(m_widgetModel->TizenId).c_str(),
                m_widgetModel->Type.Get().getApptypeToString().c_str(),
                DPL::ToUTF8String(m_widgetModel->InstallPath.Get()).c_str()
                );
    }
}

bool RunnableWidgetObject::CheckBeforeLaunch()
{
    State::StateChange change = m_guardstate->allowCheckBeforeLaunch();
    Assert(m_widgetModel);

#ifdef WRT_SMACK_ENABLED
    // TODO - this should be in the very first line of the process's main()
    // for security reasons; but for now it is easier to place here because
    // here the pkg name is already known; we don't struggle to move it
    // near the start of main() because we don't know yet if this will
    // stay in this process at all: it may be moved to AUL altogether
    set_process_config(DPL::ToUTF8String(widgetModel->TizenId).c_str());
#endif

    change.commit();
    return true;
}

bool RunnableWidgetObject::PrepareView(const std::string &startUrl,
        Evas_Object *window, Ewk_Context* ewkContext, int category)
{
    State::StateChange change = m_guardstate->allowPrepareView();
    if (!window) {
        return false;
    }

    std::string appId = DPL::ToUTF8String(m_widgetModel->TizenId);
    Try {
        if(!m_ewkContextManager) {
            m_ewkContextManager = m_contextManagerFactoryMethod(appId, ewkContext, m_view);
        } else {
            if (ewkContext &&
                    ewkContext != m_ewkContextManager->getEwkContext())
            {
                m_ewkContextManager = m_contextManagerFactoryMethod(appId, ewkContext, m_view);
            }
        }
    } Catch (DPL::Exception) {
        LogError("Internal Error during create or initialize Ewk Context");
        return false;
    }

    ADD_PROFILING_POINT("view_logic_init", "start");
    Ewk_Context* context = m_ewkContextManager->getEwkContext();

    // plugin init
    ewk_context_message_post_to_injected_bundle(
                    context,
                    MESSAGE_NAME_INITIALIZE,
                    DPL::ToUTF8String(m_widgetModel->TizenId).c_str());

    // view init
    if(!m_view->createView(context, window)) {
        return false;
    }
    m_view->prepareView(m_widgetModel.get(), startUrl, category);
    ADD_PROFILING_POINT("view_logic_init", "stop");

    change.commit();
    return true;
}

void RunnableWidgetObject::Show()
{
    State::StateChange change = m_guardstate->allowShow();

    m_view->showWidget();

    change.commit();
}

void RunnableWidgetObject::Hide()
{
    State::StateChange change = m_guardstate->allowHide();

    m_view->hideWidget();

    change.commit();
}

void RunnableWidgetObject::Suspend()
{
    LogDebug("Suspending widget");
    State::StateChange change = m_guardstate->allowSuspend();
    m_view->suspendWidget();

    change.commit();
}

void RunnableWidgetObject::Resume()
{
    LogDebug("Resuming widget");
    State::StateChange change = m_guardstate->allowResume();
    m_view->resumeWidget();

    change.commit();
}

void RunnableWidgetObject::Reset()
{
    LogDebug("Reseting widget");
    State::StateChange change = m_guardstate->allowReset();
    if (m_guardstate->toString() == "SUSPENDED") {
        m_view->resetWidgetFromSuspend();
    } else {
        // PREPARED, SECURITY_CHECKED, SHOWED
        m_view->resetWidgetFromResume();
    }

    change.commit();
}

void RunnableWidgetObject::ReloadStartPage()
{
    m_view->reloadStartPage();
}

Evas_Object* RunnableWidgetObject::GetCurrentWebview()
{
    State::StateChange change = m_guardstate->allowGetCurrentWebview();

    Evas_Object* cww = m_view->getCurrentWebview();

    change.commit();
    return cww;
}

void RunnableWidgetObject::SetUserDelegates(const UserDelegatesPtr& cbs)
{
    State::StateChange change = m_guardstate->allowSetUserDelegates();
    m_view->setUserCallbacks(cbs);
    change.commit();
}

void RunnableWidgetObject::Backward()
{
    State::StateChange change = m_guardstate->allowBackward();
    m_view->backward();

    change.commit();
}

void RunnableWidgetObject::FireJavascriptEvent(int event, void* data)
{
    State::StateChange change = m_guardstate->allowFireJavascriptEvent();
    m_view->fireJavascriptEvent(event, data);

    change.commit();
}

void RunnableWidgetObject::setViewModule(ViewModule::IViewModulePtr ptr)
{
    LogDebug("Setting ViewModule");
    m_view = ptr;
}

void RunnableWidgetObject::setContextManagerFactoryMethod(
        ViewModule::ContextManagerFactoryMethod method)
{
    LogDebug("Setting ContextManagerFactoryMethod");
    m_contextManagerFactoryMethod = method;
}

void RunnableWidgetObject::setNewState(
    std::shared_ptr<State::RunnableWidgetObjectState> sptr)
{
    LogDebug("RunnableWidgetObject changes state to: " << sptr->toString());
    m_guardstate = sptr;
}

RunnableWidgetObject::~RunnableWidgetObject()
{
    LogDebug("");
    if(m_ewkContextManager)
    {
        ewk_context_message_post_to_injected_bundle(
            m_ewkContextManager->getEwkContext(),
            MESSAGE_NAME_SHUTDOWN,
            DPL::ToUTF8String(m_widgetModel->TizenId).c_str());
    }
    else
    {
        LogError("ewk context manager is null");
    }
}
} /* namespace WRT */
