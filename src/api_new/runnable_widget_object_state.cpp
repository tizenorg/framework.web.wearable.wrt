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
 * @file    core_module.cpp
 * @author  Tomasz Iwanek (t.iwanek@samsung.com)
 * @version 1.0
 * @brief   State classes for runnable object
 */

#include "runnable_widget_object_state.h"

#include <dpl/log/log.h>

#include "runnable_widget_object.h"

namespace WRT {
namespace State {
const StateChange StateChange::NoChange = StateChange();

StateChange::StateChange()
{}

StateChange::StateChange(RunnableWidgetObjectStatePtr sptr) :
    m_sptr(sptr)
{}

void StateChange::commit()
{
    if (m_sptr) {
        m_sptr->getObject().setNewState(m_sptr);
    }
}

RunnableWidgetObjectState::RunnableWidgetObjectState(
    RunnableWidgetObject & object) :
    m_object(object)
{}

RunnableWidgetObjectState::~RunnableWidgetObjectState()
{}

RunnableWidgetObject & RunnableWidgetObjectState::getObject() const
{
    return m_object;
}

StateChange RunnableWidgetObjectState::allowCheckBeforeLaunch()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "CheckBeforeLaunch cannot be called in current state" << toString());
}

StateChange RunnableWidgetObjectState::allowPrepareView()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "PrepareView cannot be called in current state" << toString());
}

StateChange RunnableWidgetObjectState::allowShow()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Show cannot be called in current state" << toString());
}

StateChange RunnableWidgetObjectState::allowHide()
{
    return StateChange(RunnableWidgetObjectStatePtr(new HiddenState(m_object)));
}

StateChange RunnableWidgetObjectState::allowSuspend()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Hide cannot be called in current state" << toString());
}

StateChange RunnableWidgetObjectState::allowResume()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Resume cannot be called in current state" << toString());
}

StateChange RunnableWidgetObjectState::allowReset()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Cannot reset in curretn state");
}

StateChange RunnableWidgetObjectState::allowGetCurrentWebview()
{
    return StateChange::NoChange;
}

StateChange RunnableWidgetObjectState::allowSetUserDelegates()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "SetUserCallbacks cannot be called in current state");
}

StateChange RunnableWidgetObjectState::allowBackward()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Backward cannot be called in current state ");
}

StateChange RunnableWidgetObjectState::allowForward()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Foreward cannot be called in current state ");
}

StateChange RunnableWidgetObjectState::allowReload()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Reload cannot be called in current state ");
}

StateChange RunnableWidgetObjectState::allowFireJavascriptEvent()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "FireJavascriptEvent cannot be called in current state ");
}

InitialState::InitialState(RunnableWidgetObject & object) :
    RunnableWidgetObjectState(object)
{}

std::string InitialState::toString() const
{
    return "INITIAL";
}

StateChange InitialState::allowPrepareView()
{
    return StateChange(RunnableWidgetObjectStatePtr(new PreparedState(m_object)));
}

StateChange InitialState::allowHide()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Cannot hide before RunnableWidgetObject initialization");
}

StateChange InitialState::allowGetCurrentWebview()
{
    ThrowMsg(
        IRunnableWidgetObject::MethodInvocationForbidden,
        "Cannot call GetCurrentWebview before RunnableWidgetObject initialization");
}

PreparedState::PreparedState(RunnableWidgetObject & object) :
    RunnableWidgetObjectState(object)
{}

StateChange PreparedState::allowCheckBeforeLaunch()
{
    return StateChange(RunnableWidgetObjectStatePtr(new SecurityCheckedState(
                                                        m_object)));
}

std::string PreparedState::toString() const
{
    return "PREPARED";
}

SecurityCheckedState::SecurityCheckedState(RunnableWidgetObject & object) :
    RunnableWidgetObjectState(object)
{}

std::string SecurityCheckedState::toString() const
{
    return "SECURITY_CHECKED";
}

StateChange SecurityCheckedState::allowShow()
{
    return StateChange(RunnableWidgetObjectStatePtr(new ShowedState(m_object)));
}

StateChange SecurityCheckedState::allowSetUserDelegates()
{
    return StateChange::NoChange;
}

ShowedState::ShowedState(RunnableWidgetObject & object) :
    RunnableWidgetObjectState(object)
{}

std::string ShowedState::toString() const
{
    return "SHOWED";
}

StateChange ShowedState::allowSuspend()
{
    return StateChange(RunnableWidgetObjectStatePtr(new SuspendedState(m_object)));
}

StateChange ShowedState::allowBackward()
{
    return StateChange::NoChange;
}

StateChange ShowedState::allowForward()
{
    return StateChange::NoChange;
}

StateChange ShowedState::allowReload()
{
    return StateChange::NoChange;
}

StateChange ShowedState::allowReset()
{
    return StateChange::NoChange;
}

StateChange ShowedState::allowFireJavascriptEvent()
{
    return StateChange::NoChange;
}

SuspendedState::SuspendedState(RunnableWidgetObject & object) :
    RunnableWidgetObjectState(object)
{}

std::string SuspendedState::toString() const
{
    return "SUSPENDED";
}

StateChange SuspendedState::allowResume()
{
    return StateChange(RunnableWidgetObjectStatePtr(new ShowedState(m_object)));
}

StateChange SuspendedState::allowReset()
{
    return StateChange(RunnableWidgetObjectStatePtr(new ShowedState(m_object)));
}

HiddenState::HiddenState(RunnableWidgetObject & object) :
    RunnableWidgetObjectState(object)
{}

std::string HiddenState::toString() const
{
    return "HIDEN";
}

StateChange HiddenState::allowHide()
{
    ThrowMsg(IRunnableWidgetObject::MethodInvocationForbidden,
             "Hide cannot be called in current state" << toString());
}
}
}
