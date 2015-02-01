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
 * @file    core_module.cpp
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @authir  Andrzej Surdej (a.surdej@gmail.com)
 * @version 1.0
 * @brief   File contains declarations of wrt core module.
 */

#ifndef CORE_MODULE_H_
#define CORE_MODULE_H_

#include <dpl/wrt-dao-ro/wrt_db_types.h>
#include "i_runnable_widget_object.h"
#include <dpl/singleton.h>
#include <dpl/optional_typedefs.h>
#include <memory>

namespace WRT {
class CoreModuleImpl; // forward declaration

class CoreModule
{
  public:
    /**
     * Initialize needed by WRT components (database etc).
     * Will not throw exception. elm_init() is NOT called in this function.
     * You MUST call it before running widget.
     * @return true on success, false when it fails
     */
    bool Init();
    /**
     * Deinitialize CoreModule. If it called without Init() some internal
     * asserts will fail.
     */
    void Terminate();
    /**
     * Create model with given package name.
     * Init must be called earlier. You MUST destroy all
     * RunnableWidgetObjectPtr before calling Terminate.
     * @param packageName
     * @return NULL on fail
     */
    RunnableWidgetObjectPtr getRunnableWidgetObject(
        const std::string& tizenId);

  private:
    CoreModule();
    ~CoreModule();
    std::unique_ptr<CoreModuleImpl> m_impl;

    friend class DPL::Singleton<CoreModule>;
};

typedef DPL::Singleton<CoreModule> CoreModuleSingleton;
} /* namespace WRT */
#endif /* CORE_MODULE_H_ */