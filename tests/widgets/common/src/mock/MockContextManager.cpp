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
 * @file        MockContextManager.cpp
 * @author      Tomasz Iwanek (t.iwanek@samsung.com)
 * @brief       Mock for view module
 */
#include "mock/MockContextManager.h"

MockContextManager::MockContextManager(
       const std::string& tizenAppId,
       Ewk_Context* ewkContext,
       ViewModule::IViewModulePtr viewModule) :
       ViewModule::IContextManager(tizenAppId, ewkContext, viewModule)
{
}

MockContextManager::~MockContextManager()
{
}

Ewk_Context* MockContextManager::getEwkContext() const
{
    return NULL;
}

void MockContextManager::handleLowMemory()
{
}

namespace ViewModule {

ContextManagerPtr contextManagerFactoryMethod(
        const std::string& id,
        Ewk_Context* c,
        IViewModulePtr view)
{
    ContextManagerPtr ptr (new MockContextManager(id, c, view));
    return ptr;
}

ContextManagerFactoryMethod makeContextManagerFactoryMethod()
{
    return contextManagerFactoryMethod;
}

} // namespace ViewModule