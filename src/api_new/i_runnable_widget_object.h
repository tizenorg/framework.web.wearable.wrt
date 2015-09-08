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
 * @file    i_runnable_widget_object.h
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @version 1.0
 * @brief   File contains declaration of IRunnableWidgetObject interface.
 */

#ifndef RUNNABLE_WIDGET_OBJECT_INTERFACE_H_
#define RUNNABLE_WIDGET_OBJECT_INTERFACE_H_

#include <dpl/wrt-dao-ro/wrt_db_types.h>
#include <dpl/exception.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

#include <user_delegates.h>

namespace WRT {

/**
 * @brief The IRunnableWidgetObject class Runnable object interface
 *
 * Interface for managing WRT widgets runnable object.
 * Methods should be called in approopriatte order. Check graph below.
 *
 *    /----------->(INITIAL)
 *    |                |
 *    |                | PrepareView()
 *    |                V
 *    |            (PREPARED)
 *    |                |
 *    | Reset()        | CheckBeforeLaunch()
 *    |                V
 *    |           (SECCHECKED)
 *    |                |
 *    |                | Show()
 *    |                V
 *    |             (SHOWED)
 *    |               /  ^
 *    |     Suspend() |  | Resume()
 *    |               V  /
 *    |            (SUSPENDED)
 *    |
 *    |
 *    |     (any state besides INITIAL)
 *    |                |
 *    |                | Hide()
 *    |                V
 *    \-------------(HIDDEN)
 *
 */
class IRunnableWidgetObject
{
  public:
    // IRunnableWidgetObject base exception
    DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)

    // Throwed by any method if it is called in wrong state
    DECLARE_EXCEPTION_TYPE(Base, MethodInvocationForbidden)

    /**
     * Runs OCSPCheck
     * @return false when widget is already running or
     * when security check fails
     */
    virtual bool CheckBeforeLaunch() = 0;
    /**
     * Prepares view to launch. You MUST call elm_init before calling
     * this method.
     * @param window
     * @param callbacks passed to viewLogic
     */
    virtual bool PrepareView(const std::string &startUrl,
                             Evas_Object *window,
                             Ewk_Context* ewkContext = NULL,
                             int category = 0) = 0;
    /**
     * Shows widget asynchronously. Callback will be called when
     * webkit generates website.
     * @param callback
     */
    virtual void Show() = 0;
    /**
     * Hides widget. To show it again Reset must be called.
     */
    virtual void Hide() = 0;
    /**
     * Stops widget's javascript. If widget has set background_enabled
     * then this method has no effect. To resume use Resume();
     */
    virtual void Suspend() = 0;
    /**
     * Resumes widget's javascript after calling Suspend(). Resumes only if
     * widget is in suspend state.
     */
    virtual void Resume() = 0;
    /**
     * Resets widgets after calling Hide().
     */
    virtual void Reset() = 0;
    /**
     * Send Custom Event with epoch time
     */
    virtual void TimeTick(long time) = 0;
    /**
     * Send Custom Event with epoch time on the ambient mode
     */
    virtual void AmbientTick(long time) = 0;
    /**
     * Send Custom Event to notify ambient mode changed
     */
    virtual void AmbientModeChanged(bool ambient_mode) = 0;
    /**
     * Reload start page on widget.
     */
    virtual void ReloadStartPage() = 0;
    /**
     * Retrieve widget's top level webview
     * @return Evas_Object*
     */
    virtual Evas_Object* GetCurrentWebview() = 0;
    /**
     * Register widget's delegates
     */
    virtual void SetUserDelegates(const UserDelegatesPtr& cbs) = 0;
    /**
     * Call goBack() on webkit
     */
    virtual void Backward() = 0;
    /**
     * fire custom javascript event
     */
    virtual void FireJavascriptEvent(int event, void* data) = 0;

    virtual ~IRunnableWidgetObject() {}
};

typedef std::shared_ptr<IRunnableWidgetObject> RunnableWidgetObjectPtr;
}
#endif /* RUNNABLE_WIDGET_OBJECT_INTERFACE_H_ */

