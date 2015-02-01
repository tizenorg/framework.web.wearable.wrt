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
 * @file    TestCases.cpp
 * @author  Karol Pawłowski (k.pawlowski@samsung.com)
 * @author  Tomasz Iwanek (t.iwanek@samsung.com)
 * @version 1.0
 */

#include <dpl/test/test_runner.h>
#include <dpl/singleton_impl.h>
#include <dpl/log/log.h>
#include <dpl/lexical_cast.h>
#include <dpl/abstract_waitable_input_adapter.h>
#include <dpl/test/value_separated_reader.h>
#include <dpl/test/process_pipe.h>
#include <RunnableObjectStateTester.h>
#include <i_runnable_widget_object.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>

#include <memory>
#include <vector>
#include <string>

namespace {

struct Result {
    bool m_exc;
    bool m_exd;
    bool m_exs;
    std::string message;
    Result(bool exc = false, bool exd = false, bool exs = false)
        : m_exc(exc), m_exd(exd), m_exs(exs) {}
};

}

RUNNER_TEST_GROUP_INIT(RunnableWidgetObjectState)

//for call methods to get starting state or do clean up after test
#define CALL_TESTER( ARGUMENT )                                        \
        tester.perform##ARGUMENT();                                    \

//for performing test and catch excpeted and unexpected exceptions
#define RUNNABLE_TESTER( ARGUMENT )                                    \
    Result res;                                                        \
    Try                                                                \
    {                                                                  \
        LogDebug("perform start");                                     \
        CALL_TESTER( ARGUMENT )                                        \
        LogDebug("perform stop");                                      \
    }                                                                  \
    Catch(WRT::IRunnableWidgetObject::MethodInvocationForbidden)       \
    {                                                                  \
        res.m_exc = true;                                              \
        res.message = _rethrown_exception.DumpToString();              \
        res.message += "(MethodInvocationForbidden)";                  \
    }                                                                  \
    Catch(DPL::Exception)                                              \
    {                                                                  \
        res.m_exd = true;                                              \
        res.message = _rethrown_exception.DumpToString();              \
        res.message += "(DPL::Exception)";                             \
    }                                                                  \
    Catch(std::exception)                                              \
    {                                                                  \
        res.m_exs = true;                                              \
        res.message = _rethrown_exception.what();                      \
        res.message += "(std::exception)";                             \
    }                                                                  \


//check of function success
#define SHOULD_BE_ALLOWED( ARGUMENT )                                  \
    {                                                                  \
    RUNNABLE_TESTER( ARGUMENT )                                        \
    if(res.m_exc || res.m_exs || res.m_exd)                            \
    {                                                                  \
        ok = false;                                                    \
        reason = "Exception throwed when not expected in ";            \
        reason += __FUNCTION__;                                        \
        reason += " calling ";                                         \
        reason += #ARGUMENT;                                           \
        reason += "message ";                                          \
        reason += res.message;                                         \
    }                                                                  \
    }                                                                  \


//check of function failure
#define SHOULD_BE_FORBIDDEN( ARGUMENT )                                \
    {                                                                  \
    RUNNABLE_TESTER( ARGUMENT )                                        \
    if(!res.m_exc)                                                     \
    {                                                                  \
        ok = false;                                                    \
        reason = "MethodInvocationForbidden should be throwed";        \
        reason += " when calling ";                                    \
        reason += #ARGUMENT;                                           \
    }                                                                  \
    }                                                                  \


//marcos for hiding lamdba expression and and presenting it as if it is test body
#define RUNNABLE_TESTER_START                                                              \
    bool ok = true;                                                                        \
    std::string reason;                                                                    \
    auto func = [&ok,&reason](RunnableObjectStateTester & tester)                          \
    {                                                                                      \
        Try                                                                                \
        {                                                                                  \


#define RUNNABLE_TESTER_STOP                                                               \
        }                                                                                  \
        Catch(DPL::Exception)                                                              \
        {                                                                                  \
            ok = false;                                                                    \
            reason = _rethrown_exception.DumpToString();                                   \
            reason += "(DPL::Exception)";                                                  \
        }                                                                                  \
        Catch(std::exception)                                                              \
        {                                                                                  \
            reason = "Unknown exception";                                                  \
            ok = false;                                                                    \
        }                                                                                  \
    };                                                                                     \
    RunnableObjectStateTester & instance = RunnableObjectStateTesterSingleton::Instance(); \
    instance.runTest(func);                                                                \
    RUNNER_ASSERT_MSG(ok, reason);                                                         \

//Initial
/*
Name: state_Initial_CheckBeforeLaunch
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method CheckBeforeLaunch in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_CheckBeforeLaunch)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( CheckBeforeLaunch );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_PrepareView
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method PrepareView in state Initial
Expected: Transistion should pass
*/
RUNNER_TEST(state_Initial_PrepareView)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_ALLOWED( PrepareView );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_Show
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Show in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_Show)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( Show );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_Hide
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Hide in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_Hide)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_Suspend
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Suspend in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_Suspend)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( Suspend );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_Resume
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Resume in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_Resume)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( Resume );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_Reset
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Reset in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_Reset)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( Reset );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_GetCurrentWebview
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method GetCurrentWebview in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_GetCurrentWebview)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( GetCurrentWebview );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_SetUserDelegates
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method SetUserDelegates in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_SetUserDelegates)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( SetUserDelegates );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Initial_Backward
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Backward in state Initial
Expected: Transistion should fail
*/
RUNNER_TEST(state_Initial_Backward)
{
    RUNNABLE_TESTER_START
    SHOULD_BE_FORBIDDEN( Backward );
    RUNNABLE_TESTER_STOP
}


//Prepared
/*
Name: state_Prepared_CheckBeforeLaunch
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method CheckBeforeLaunch in state Prepared
Expected: Transistion should pass
*/
RUNNER_TEST(state_Prepared_CheckBeforeLaunch)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_ALLOWED( CheckBeforeLaunch );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_PrepareView
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method PrepareView in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_PrepareView)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( PrepareView );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_Show
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Show in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_Show)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( Show );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_Hide
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Hide in state Prepared
Expected: Transistion should pass
*/
RUNNER_TEST(state_Prepared_Hide)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_ALLOWED( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_Suspend
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Suspend in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_Suspend)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( Suspend );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_Resume
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Resume in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_Resume)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( Resume );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_Reset
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Reset in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_Reset)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( Reset );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_GetCurrentWebview
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method GetCurrentWebview in state Prepared
Expected: Transistion should pass
*/
RUNNER_TEST(state_Prepared_GetCurrentWebview)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_ALLOWED( GetCurrentWebview );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_SetUserDelegates
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method SetUserDelegates in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_SetUserDelegates)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( SetUserDelegates );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Prepared_Backward
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Backward in state Prepared
Expected: Transistion should fail
*/
RUNNER_TEST(state_Prepared_Backward)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    SHOULD_BE_FORBIDDEN( Backward );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

//SecChecked
/*
Name: state_SecChecked_CheckBeforeLaunch
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method CheckBeforeLaunch in state SecChecked
Expected: Transistion should fail
*/
RUNNER_TEST(state_SecChecked_CheckBeforeLaunch)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_FORBIDDEN( CheckBeforeLaunch );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_PrepareView
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method PrepareView in state SecChecked
Expected: Transistion should fail
*/
RUNNER_TEST(state_SecChecked_PrepareView)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_FORBIDDEN( PrepareView );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_Show
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Show in state SecChecked
Expected: Transistion should pass
*/
RUNNER_TEST(state_SecChecked_Show)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( SetUserDelegates );
    SHOULD_BE_ALLOWED( Show );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_Hide
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Hide in state SecChecked
Expected: Transistion should pass
*/
RUNNER_TEST(state_SecChecked_Hide)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_ALLOWED( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_Suspend
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Suspend in state SecChecked
Expected: Transistion should fail
*/
RUNNER_TEST(state_SecChecked_Suspend)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_FORBIDDEN( Suspend );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_Resume
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Resume in state SecChecked
Expected: Transistion should fail
*/
RUNNER_TEST(state_SecChecked_Resume)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_FORBIDDEN( Resume );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_Reset
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Reset in state SecChecked
Expected: Transistion should fail
*/
RUNNER_TEST(state_SecChecked_Reset)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_FORBIDDEN( Reset );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_GetCurrentWebview
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method GetCurrentWebview in state SecChecked
Expected: Transistion should pass
*/
RUNNER_TEST(state_SecChecked_GetCurrentWebview)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_ALLOWED( GetCurrentWebview );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_SetUserDelegates
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method SetUserDelegates in state SecChecked
Expected: Transistion should pass
*/
RUNNER_TEST(state_SecChecked_SetUserDelegates)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_ALLOWED( SetUserDelegates );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_SecChecked_Backward
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Backward in state SecChecked
Expected: Transistion should fail
*/
RUNNER_TEST(state_SecChecked_Backward)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    SHOULD_BE_FORBIDDEN( Backward );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

//Showed
/*
Name: state_Showed_CheckBeforeLaunch
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method CheckBeforeLaunch in state Showed
Expected: Transistion should fail
*/
RUNNER_TEST(state_Showed_CheckBeforeLaunch)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_FORBIDDEN( CheckBeforeLaunch );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_PrepareView
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method PrepareView in state Showed
Expected: Transistion should fail
*/
RUNNER_TEST(state_Showed_PrepareView)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_FORBIDDEN( PrepareView );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_Show
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Show in state Showed
Expected: Transistion should fail
*/
RUNNER_TEST(state_Showed_Show)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_FORBIDDEN( Show );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_Hide
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Hide in state Showed
Expected: Transistion should pass
*/
RUNNER_TEST(state_Showed_Hide)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_ALLOWED( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_Suspend
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Suspend in state Showed
Expected: Transistion should pass
*/
RUNNER_TEST(state_Showed_Suspend)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_ALLOWED( Suspend );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_Resume
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Resume in state Showed
Expected: Transistion should fail
*/
RUNNER_TEST(state_Showed_Resume)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_FORBIDDEN( Resume );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_Reset
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Reset in state Showed
Expected: Transistion should pass
*/
RUNNER_TEST(state_Showed_Reset)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_ALLOWED( Reset );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_GetCurrentWebview
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method GetCurrentWebview in state Showed
Expected: Transistion should pass
*/
RUNNER_TEST(state_Showed_GetCurrentWebview)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_ALLOWED( GetCurrentWebview );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_SetUserDelegates
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method SetUserDelegates in state Showed
Expected: Transistion should fail
*/
RUNNER_TEST(state_Showed_SetUserDelegates)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_FORBIDDEN( SetUserDelegates );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Showed_Backward
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Backward in state Showed
Expected: Transistion should pass
*/
RUNNER_TEST(state_Showed_Backward)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    SHOULD_BE_ALLOWED( Backward );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

//Suspended
/*
Name: state_Suspended_CheckBeforeLaunch
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method CheckBeforeLaunch in state Suspended
Expected: Transistion should fail
*/
RUNNER_TEST(state_Suspended_CheckBeforeLaunch)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_FORBIDDEN( CheckBeforeLaunch );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_PrepareView
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method PrepareView in state Suspended
Expected: Transistion should fail
*/
RUNNER_TEST(state_Suspended_PrepareView)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_FORBIDDEN( PrepareView );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_Show
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Show in state Suspended
Expected: Transistion should fail
*/
RUNNER_TEST(state_Suspended_Show)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_FORBIDDEN( Show );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_Hide
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Hide in state Suspended
Expected: Transistion should pass
*/
RUNNER_TEST(state_Suspended_Hide)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_ALLOWED( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_Suspend
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Suspend in state Suspended
Expected: Transistion should fail
*/
RUNNER_TEST(state_Suspended_Suspend)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_FORBIDDEN( Suspend );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_Resume
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Resume in state Suspended
Expected: Transistion should pass
*/
RUNNER_TEST(state_Suspended_Resume)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_ALLOWED( Resume );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_Reset
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Reset in state Suspended
Expected: Transistion should pass
*/
RUNNER_TEST(state_Suspended_Reset)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_ALLOWED( Reset );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_GetCurrentWebview
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method GetCurrentWebview in state Suspended
Expected: Transistion should pass
*/
RUNNER_TEST(state_Suspended_GetCurrentWebview)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_ALLOWED( GetCurrentWebview );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_SetUserDelegates
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method SetUserDelegates in state Suspended
Expected: Transistion should fail
*/
RUNNER_TEST(state_Suspended_SetUserDelegates)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_FORBIDDEN( SetUserDelegates );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Suspended_Backward
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Backward in state Suspended
Expected: Transistion should fail
*/
RUNNER_TEST(state_Suspended_Backward)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    SHOULD_BE_FORBIDDEN( Backward );
    CALL_TESTER( Hide );
    RUNNABLE_TESTER_STOP
}

//Hidden
/*
Name: state_Hidden_CheckBeforeLaunch
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method CheckBeforeLaunch in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_CheckBeforeLaunch)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( CheckBeforeLaunch );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_PrepareView
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method PrepareView in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_PrepareView)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( PrepareView );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_Show
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Show in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_Show)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( Show );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_Hide
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Hide in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_Hide)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( Hide );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_Suspend
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Suspend in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_Suspend)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( Suspend );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_Resume
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Resume in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_Resume)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( Resume );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_Reset
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Reset in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_Reset)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( Reset );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_GetCurrentWebview
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method GetCurrentWebview in state Hidden
Expected: Transistion should pass
*/
RUNNER_TEST(state_Hidden_GetCurrentWebview)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_ALLOWED( GetCurrentWebview );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_SetUserDelegates
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method SetUserDelegates in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_SetUserDelegates)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( SetUserDelegates );
    RUNNABLE_TESTER_STOP
}

/*
Name: state_Hidden_Backward
Description: Checks if RunnableWidgetObject
 correctly reacts to calling method Backward in state Hidden
Expected: Transistion should fail
*/
RUNNER_TEST(state_Hidden_Backward)
{
    RUNNABLE_TESTER_START
    CALL_TESTER( PrepareView );
    CALL_TESTER( CheckBeforeLaunch );
    CALL_TESTER( Show );
    CALL_TESTER( Suspend );
    CALL_TESTER( Hide );
    SHOULD_BE_FORBIDDEN( Backward );
    RUNNABLE_TESTER_STOP
}


RUNNER_TEST_GROUP_INIT(CommandLineParameters)


struct PassAllParserPolicy
{
    static bool SkipLine(const std::vector<std::string> & ) { return false; }
    static bool Validate(std::shared_ptr<std::vector<std::vector<std::string> > > &) { return true; }
};

struct ListTokenizerPolicy
{
    static std::string GetSeperators() { return " "; }
    static bool SkipEmpty() { return true; }
    static void PrepareValue(std::string & value) { DPL::Trim(value, " "); }
    static bool TryAgainAtEnd(int) { return false; }
};

typedef DPL::VSReader<PassAllParserPolicy, ListTokenizerPolicy> LListReader;

RUNNER_TEST(CommandLineParameters_ListWidgets)
{
    int status = system("wrt_reset_all.sh");
    RUNNER_ASSERT_MSG(WIFEXITED(status) == TRUE, "wrt_reset_all.sh not exited properly");
    RUNNER_ASSERT_MSG(WEXITSTATUS(status) == 0, "wrt_reset_all.sh failed");

    //reset database connection
    WrtDB::WrtDatabase::detachFromThread();
    WrtDB::WrtDatabase::attachToThreadRW();

    std::string tizenId;
    RUNNER_ASSERT(InstallerWrapper::install("/opt/share/widget/tests/general/minimal.wgt", tizenId) == InstallerWrapper::Success);

    try
    {
        DPL::ProcessPipe pipe(DPL::ProcessPipe::PipeErrorPolicy::OFF);
        pipe.Open("DPL_USE_OLD_STYLE_LOGS=0 wrt-launcher -l");
        LListReader launcher(std::make_shared<DPL::AbstractWaitableInputAdapter>(&pipe));
        DPL::VSResultPtr result = launcher.ReadInput();
        pipe.Close();

        RUNNER_ASSERT_MSG((*result)[2][0] == "1", " Wrong position number");
        RUNNER_ASSERT_MSG((*result)[2][1] == "Minimal", "Wrong name");
        RUNNER_ASSERT_MSG((*result)[2][2] == "1.0", "Wrong version");
        RUNNER_ASSERT_MSG((*result)[2][3] == "http://test.samsung.com/widget/minimalTest", "Wrong GUID");
        RUNNER_ASSERT_MSG((*result)[2][4] == "aHFQwMDkmC", "Wrong pkgID");
        RUNNER_ASSERT_MSG((*result)[2][5] == "aHFQwMDkmC.minimal", "Wrong appID");
    }
    catch(...)
    {
        RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
        throw;
    }
    RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
}


struct ResultTokenizerPolicy
{
    static std::string GetSeperators() { return ":"; }
    static bool SkipEmpty() { return true; }
    static void PrepareValue(std::string & value) { DPL::Trim(value, " "); }
    static bool TryAgainAtEnd(int) { return false; }
};

typedef DPL::VSReader<PassAllParserPolicy, ResultTokenizerPolicy> LResultReader;

RUNNER_TEST(CommandLineParameters_SimpleLaunch)
{
    bool launched = false;
    bool running = false;
    bool stopped = false;
    std::string tizenId;
    RUNNER_ASSERT(InstallerWrapper::install("/opt/share/widget/tests/general/minimal.wgt", tizenId) == InstallerWrapper::Success);

    //start
    {
        DPL::ProcessPipe pipe(DPL::ProcessPipe::PipeErrorPolicy::OFF);
        pipe.Open(std::string("DPL_USE_OLD_STYLE_LOGS=0 wrt-launcher -s ") + tizenId);
        LResultReader launcher(std::make_shared<DPL::AbstractWaitableInputAdapter>(&pipe));
        DPL::VSResultPtr result = launcher.ReadInput();
        pipe.Close();

        FOREACH(row, *result)
        {
            if(row->at(0) == "result")
            {
                if(row->at(1) == "launched")
                {
                    launched = true;
                    break;
                }
                else
                {
                    RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
                    RUNNER_ASSERT_MSG(false, "Launch failed");
                }
            }
        }
    }
    if(!launched)
    {
        RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
        RUNNER_ASSERT_MSG(false, "Result not returned");
    }

    //is-running
    {
        DPL::ProcessPipe pipe(DPL::ProcessPipe::PipeErrorPolicy::OFF);
        pipe.Open(std::string("DPL_USE_OLD_STYLE_LOGS=0 wrt-launcher -r ") + tizenId);
        LResultReader launcher(std::make_shared<DPL::AbstractWaitableInputAdapter>(&pipe));
        DPL::VSResultPtr result = launcher.ReadInput();
        pipe.Close();

        FOREACH(row, *result)
        {
            if(row->at(0) == "result")
            {
                if(row->at(1) == "running")
                {
                    running = true;
                    break;
                }
                else
                {
                    RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
                    RUNNER_ASSERT_MSG(false, "App launched but not running");
                }
            }
        }
    }
    if(!running)
    {
        RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
        RUNNER_ASSERT_MSG(false, "Result not returned");
    }

    //stop
    {
        DPL::ProcessPipe pipe(DPL::ProcessPipe::PipeErrorPolicy::OFF);
        pipe.Open(std::string("DPL_USE_OLD_STYLE_LOGS=0 wrt-launcher -k ") + tizenId);
        LResultReader launcher(std::make_shared<DPL::AbstractWaitableInputAdapter>(&pipe));
        DPL::VSResultPtr result = launcher.ReadInput();
        pipe.Close();

        FOREACH(row, *result)
        {
            if(row->at(0) == "result")
            {
                if(row->at(1) == "killed")
                {
                    stopped = true;
                    break;
                }
                else
                {
                    RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
                    RUNNER_ASSERT_MSG(false, "Stop failed");
                }
            }
        }
    }
    RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
    if(stopped)
    {
        return;
    }
    RUNNER_ASSERT_MSG(false, "Result not returned");
}

RUNNER_TEST(CommandLineParameters_WebDebuggerPort)
{
    std::string tizenId;
    RUNNER_ASSERT(InstallerWrapper::install("/opt/share/widget/tests/general/minimal.wgt", tizenId) == InstallerWrapper::Success);

    DPL::ProcessPipe pipe(DPL::ProcessPipe::PipeErrorPolicy::OFF);
    pipe.Open(std::string("DPL_USE_OLD_STYLE_LOGS=0 wrt-launcher -d -t 5 -s ") + tizenId);
    LResultReader launcher(std::make_shared<DPL::AbstractWaitableInputAdapter>(&pipe));
    DPL::VSResultPtr result = launcher.ReadInput();
    pipe.Close();

    FOREACH(row, *result)
    {
        if(row->at(0) == "port")
        {
            int value = DPL::lexical_cast<int>(row->at(1));
            if(value >= 1024 && value <= 65535)
            {
                RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
                return;
            }
            else
            {
                RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
                RUNNER_ASSERT_MSG(false, "Wrong port number");
            }
        }
    }
    RUNNER_ASSERT(InstallerWrapper::uninstall(tizenId));
    RUNNER_ASSERT_MSG(false, "Port number not returned");
}

