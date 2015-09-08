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
 * @file        MockViewModule.cpp
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       Mock for view module
 */

#include "mock/MockViewModule.h"

MockViewModule::MockViewModule()
{
}

bool MockViewModule::createView(
        Ewk_Context* /*context*/,
        Evas_Object* /*window*/)
{
    return true;
}

void MockViewModule::prepareView(WidgetModel *, const std::string &)
{
}

void MockViewModule::showWidget()
{
}

void MockViewModule::hideWidget()
{
}

void MockViewModule::suspendWidget()
{
}

void MockViewModule::resumeWidget()
{
}

void MockViewModule::resetWidgetFromSuspend()
{
}

void MockViewModule::resetWidgetFromResume()
{
}

void MockViewModule::backward()
{
}

void MockViewModule::reloadStartPage(bool isbackground)
{
}

Evas_Object* MockViewModule::getCurrentWebview()
{
    return NULL;
}

void MockViewModule::fireJavascriptEvent(int /*event*/, void* /*data*/)
{
}

void MockViewModule::setUserCallbacks(const WRT::UserDelegatesPtr& /*cbs*/)
{
}

void MockViewModule::checkSyncMessageFromBundle(
        const char* /*name*/,
        const char* /*body*/,
        char** /*returnData*/)
{
}

void MockViewModule::checkAsyncMessageFromBundle(
        const char* /*name*/,
        const char* /*body*/)
{
}

void MockViewModule::downloadData(const char* /*url*/)
{
}

void MockViewModule::activateVibration(bool /*on*/, uint64_t /*time*/)
{
}
