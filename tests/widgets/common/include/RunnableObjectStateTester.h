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
 * @file        RunnableObjectStateTester.h
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       Tester for RunnableWidgetObject's states
 */

#ifndef WRT_EXTRA_AUTO_TESTS_COMMON_INCLUDE_RUNNABLE_OBJECT_STATE_TESTER_H
#define WRT_EXTRA_AUTO_TESTS_COMMON_INCLUDE_RUNNABLE_OBJECT_STATE_TESTER_H

#include <memory>
#include <functional>
#include <vector>
#include <Evas.h>
#include <dpl/exception.h>
#include <dpl/application.h>
#include <dpl/generic_event.h>
#include <dpl/event/controller.h>
#include <dpl/type_list.h>
#include <dpl/task.h>
#include <dpl/singleton.h>
#include <dpl/string.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <runnable_widget_object.h>
#include <core_module.h>

#include <InstallerWrapper.h>

DECLARE_GENERIC_EVENT_0(NextStepEvent)

class RunnableObjectStateTester : public DPL::ApplicationExt,
        private DPL::Event::Controller<DPL::TypeListDecl<NextStepEvent>::Type>,
        public DPL::TaskDecl<RunnableObjectStateTester>
{
public:
    typedef std::function<void(RunnableObjectStateTester&)> Test;

    DECLARE_EXCEPTION_TYPE(DPL::Exception, Base)
    DECLARE_EXCEPTION_TYPE(Base, CoreModuleFailure)
    DECLARE_EXCEPTION_TYPE(Base, CallbackFailure)

    RunnableObjectStateTester();
    ~RunnableObjectStateTester();

    void performCheckBeforeLaunch();
    void performPrepareView();

    void performShow();
    void performHide();
    void performSuspend();
    void performResume();
    void performReset();

    void performGetCurrentWebview();
    void performSetUserDelegates();
    void performBackward();

    void performTest();

    void runTest(Test func);

    void OnEventReceived(const NextStepEvent& /*event*/);

    void loadFinishCallback(Evas_Object* obj, void* eventInfo);

private:
    std::string m_handle;
    WRT::RunnableWidgetObjectPtr m_widget;
    Test m_func;
    const std::string m_widgetPath;
};

typedef DPL::Singleton<RunnableObjectStateTester> RunnableObjectStateTesterSingleton;

#endif // WRT_EXTRA_AUTO_TESTS_COMMON_INCLUDE_RUNNABLE_OBJECT_STATE_TESTER_H
