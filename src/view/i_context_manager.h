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
 * @file    i_context_manager.h
 * @author  Iwanek Tomasz (t.iwanek@samsung.com)
 * @version 0.1
 * @brief   Abstract file for handling operation regarding Ewk_Context.
 */

#ifndef ABSTRACT_CONTEXT_MANAGER_H
#define ABSTRACT_CONTEXT_MANAGER_H

#include <memory>
#include <functional>
#include <EWebKit.h>
#include <EWebKit_internal.h>

#include <i_view_module.h>

namespace ViewModule {

class IContextManager;
typedef std::shared_ptr<IContextManager> ContextManagerPtr;

typedef std::function<ContextManagerPtr (const std::string&, Ewk_Context*, ViewModule::IViewModulePtr)> ContextManagerFactoryMethod;

/**
 * @brief The AbstractContextManager class Factory for ewk context
 *
 * This is interface class for ewk context factory.
 * It's uses tizenId, view module to initialize it approriatly context.
 *
 * Constructor should create new context only if ewkContext parameter is NULL.
 * If ewkContext parameter is not NULL, context should not be destroyed in destructor.
 * This means used context is managed by manager only if was created internally.
 *
 * NOTE: This interface in not visible outside core module and it should not be.
 *       Reason for this code is not modify RunnableWidgetObject behaviour for mocks.
 */
class IContextManager {
public:
    IContextManager(
            const std::string& tizenAppId,
            Ewk_Context* ewkContext,
            ViewModule::IViewModulePtr viewModule) :
            m_appId(tizenAppId), m_ewkContext(ewkContext), m_view(viewModule) {}
    virtual ~IContextManager() {}
    /**
     * @brief getEwkContext returns ewk context
     * @return ewk context
     */
    virtual Ewk_Context* getEwkContext() const = 0;
    /**
     * @brief handleLowMemory
     *
     * Handles low memory conditions
     */
    virtual void handleLowMemory() = 0;
protected:
    std::string m_appId;
    Ewk_Context* m_ewkContext;
    IViewModulePtr m_view;
};

ContextManagerPtr contextManagerFactoryMethod(const std::string& id, Ewk_Context* c, IViewModulePtr view);

ContextManagerFactoryMethod makeContextManagerFactoryMethod();

} // namespace ViewModule

#endif // ABSTRACT_CONTEXT_MANAGER_H
