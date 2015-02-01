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
#ifndef RUNNABLE_WIDGET_OBJECT_STATE_H
#define RUNNABLE_WIDGET_OBJECT_STATE_H

//forward declarations
namespace WRT {
class RunnableWidgetObject;

namespace State {
class RunnableWidgetObjectState;
}
}

#include <dpl/exception.h>

#include <i_runnable_widget_object.h>

namespace WRT {
namespace State {
typedef std::shared_ptr<RunnableWidgetObjectState> RunnableWidgetObjectStatePtr;

/**
 * @brief The StateChange class
 *
 * RunnableWidgetObject state change abstraction
 */
class StateChange
{
  public:
    static const StateChange NoChange;

    StateChange();
    explicit StateChange(RunnableWidgetObjectStatePtr sptr);

    /**
     * @brief commit actually performs change of state
     */
    void commit();

  private:
    RunnableWidgetObjectStatePtr m_sptr;
};

/**
 * @brief The RunnableWidgetObjectState class
 *
 * Base class for all runnable object states
 *
 * Allow methods should be called if referenced method are actually
 * going to be called effectively. They return next state object.
 * Call commit on this object to make change of state of RunnableWidgetObject
 */
class RunnableWidgetObjectState
{
  public:
    explicit RunnableWidgetObjectState(RunnableWidgetObject & object);
    virtual ~RunnableWidgetObjectState();

    virtual StateChange allowCheckBeforeLaunch();
    virtual StateChange allowPrepareView();
    virtual StateChange allowShow();
    virtual StateChange allowHide();
    virtual StateChange allowSuspend();
    virtual StateChange allowResume();
    virtual StateChange allowReset();
    virtual StateChange allowGetCurrentWebview();
    virtual StateChange allowSetUserDelegates();
    virtual StateChange allowBackward();
    virtual StateChange allowForward();
    virtual StateChange allowReload();
    virtual StateChange allowFireJavascriptEvent();

    virtual std::string toString() const = 0;
    virtual RunnableWidgetObject & getObject() const;

  protected:
    RunnableWidgetObject & m_object;
};

/**
 * INITIAL STATE
 */
class InitialState : public RunnableWidgetObjectState
{
  public:
    explicit InitialState(RunnableWidgetObject & object);
    std::string toString() const;

    StateChange allowPrepareView();
    StateChange allowHide();
    StateChange allowGetCurrentWebview();
};

/**
 * PREPARED STATE
 */
class PreparedState : public RunnableWidgetObjectState
{
  public:
    explicit PreparedState(RunnableWidgetObject & object);
    std::string toString() const;

    StateChange allowCheckBeforeLaunch();
};

/**
 * SECURITY CHECKED STATE
 */
class SecurityCheckedState : public RunnableWidgetObjectState
{
  public:
    explicit SecurityCheckedState(RunnableWidgetObject & object);
    std::string toString() const;

    StateChange allowShow();
    StateChange allowSetUserDelegates();
};

/**
 * SHOWED STATE
 */
class ShowedState : public RunnableWidgetObjectState
{
  public:
    explicit ShowedState(RunnableWidgetObject & object);
    std::string toString() const;

    StateChange allowSuspend();
    StateChange allowBackward();
    StateChange allowForward();
    StateChange allowReload();
    StateChange allowReset();
    StateChange allowFireJavascriptEvent();
};

/**
 * SUSPENDED STATE
 */
class SuspendedState : public RunnableWidgetObjectState
{
  public:
    explicit SuspendedState(RunnableWidgetObject & object);
    std::string toString() const;

    StateChange allowResume();
    StateChange allowReset();
};

/**
 * HIDDEN STATE
 */
class HiddenState : public RunnableWidgetObjectState
{
  public:
    explicit HiddenState(RunnableWidgetObject & object);
    std::string toString() const;

    StateChange allowHide();
};
}
}

#endif // RUNNABLE_WIDGET_OBJECT_STATE_H
