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
 * @file        RunnableObjectStateTester.cpp
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       Tester for RunnableWidgetObject's states
 */

#include "RunnableObjectStateTester.h"

#include <functional>
#include <vector>
#include <string>
#include <dpl/framework_efl.h>
#include <dpl/localization/w3c_file_localization.h>
#include <dpl/availability.h>
#include <dpl/noncopyable.h>
#include <dpl/log/log.h>
#include <dpl/singleton_impl.h>
#include <dpl/foreach.h>
#include <user_delegates.h>

#include <mock/MockViewModule.h>
#include <mock/MockContextManager.h>

IMPLEMENT_SINGLETON(RunnableObjectStateTester)

namespace {
const std::string widgetPathMnimal = "/opt/share/widget/tests/general/any.wgt";
const int widgetInitializationTimeBounding = 3;
}

RunnableObjectStateTester::RunnableObjectStateTester()
    : DPL::ApplicationExt(1, NULL, "test-app"),
      DPL::TaskDecl<RunnableObjectStateTester>(this),
      m_widgetPath(widgetPathMnimal)
{
    LogDebug("enter");

    int argc = 0;
    const char * argv[] = { "wrt-tests-wrt" };
    if (!getenv("ELM_ENGINE")) {
        if (setenv("ELM_ENGINE", "gl", 1)) {
                LogDebug("Enable backend");
        }
    }
    setenv("COREGL_FASTPATH", "1", 1);
    DPL::Log::LogSystemSingleton::Instance().SetTag("WRT-CLIENT");

    Touch();

    //initialize
    elm_init(argc, const_cast<char**>(argv));
    if(!WRT::CoreModuleSingleton::Instance().Init())
    {
        ThrowMsg(CoreModuleFailure, "Init() fails");
    }

    InstallerWrapper::install(m_widgetPath, m_handle);
    LogDebug("Widget Installed: " << m_handle);

    AddStep(&RunnableObjectStateTester::performTest);
}

RunnableObjectStateTester::~RunnableObjectStateTester()
{
    LogDebug("enter");
    WRT::CoreModuleSingleton::Instance().Terminate();
    InstallerWrapper::uninstall(m_handle);
    elm_shutdown();
}

void RunnableObjectStateTester::performCheckBeforeLaunch()
{
    m_widget->CheckBeforeLaunch();
}

void RunnableObjectStateTester::performPrepareView()
{
    //this address is invalid but we use mock (may be problematic when implementation'll change)
    Evas_Object * fakeWindowAddress = reinterpret_cast<Evas_Object*>(0x01);
    m_widget->PrepareView(DPL::ToUTF8String(DPL::String()),
            fakeWindowAddress);
}

void RunnableObjectStateTester::performShow()
{
    m_widget->Show();
}

void RunnableObjectStateTester::performHide()
{
    m_widget->Hide();
}

void RunnableObjectStateTester::performSuspend()
{
    m_widget->Suspend();
}

void RunnableObjectStateTester::performResume()
{
    m_widget->Resume();
}

void RunnableObjectStateTester::performReset()
{
    m_widget->Reset();
}

void RunnableObjectStateTester::performGetCurrentWebview()
{
    m_widget->GetCurrentWebview();
}

void RunnableObjectStateTester::performSetUserDelegates()
{
    WRT::UserDelegatesPtr dlgs(new WRT::UserDelegates);
    dlgs->loadFinishedCallback =
        std::bind(&RunnableObjectStateTester::loadFinishCallback,
                  this,
                  std::placeholders::_1,
                  std::placeholders::_2);
    m_widget->SetUserDelegates(dlgs);
}

void RunnableObjectStateTester::performBackward()
{
    m_widget->Backward();
}

void RunnableObjectStateTester::OnEventReceived(const NextStepEvent& /*event*/)
{
    NextStep();
}

void RunnableObjectStateTester::loadFinishCallback(Evas_Object* obj, void* eventInfo)
{
    DPL_UNUSED_PARAM(obj);
    DPL_UNUSED_PARAM(eventInfo);

    LogDebug("enter");
}

void RunnableObjectStateTester::runTest(RunnableObjectStateTester::Test func)
{
    m_widget = WRT::CoreModuleSingleton::Instance().getRunnableWidgetObject(m_handle);
    //as we know we uses specific implementation
    WRT::RunnableWidgetObject * m_widget_impl = dynamic_cast<WRT::RunnableWidgetObject*>(m_widget.get());
    Assert(m_widget_impl);
    m_widget_impl->setViewModule(ViewModule::IViewModulePtr(new MockViewModule()));
    m_widget_impl->setContextManagerFactoryMethod(ViewModule::makeContextManagerFactoryMethod());
    if(!m_widget)
    {
        ThrowMsg(CoreModuleFailure, "getRunnableWidgetObject() fails");
    }

    m_func = func;
    SwitchToStep(&RunnableObjectStateTester::performTest);
    DPL::Event::ControllerEventHandler<NextStepEvent>::PostTimedEvent(NextStepEvent(), 1.0); //TODO: time hazard

    elm_run(); //elm_run should be called instead of ecore_main_loop
}

void RunnableObjectStateTester::performTest()
{
    LogDebug("enter");

    Try {
        m_func(*this);
    } Catch(DPL::Exception) {
        LogDebug("Test broken not at condition to be checked. Check other tests");
        LogDebug(_rethrown_exception.DumpToString());
    }

    Quit();
    m_widget.reset();

    LogDebug("leave");
}
