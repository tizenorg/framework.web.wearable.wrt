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
 * @file    ewk_context_manager.h
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @version 0.1
 * @brief   Declaration of EwkContextManager class.
 *          This file handles operation regarding Ewk_Context.
 */

#ifndef EWK_CONTEXT_MANAGER_H
#define EWK_CONTEXT_MANAGER_H

#include <cstdint>
#include <string>

#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <vconf.h>
#include <i_context_manager.h>
#include <i_view_module.h>

namespace ViewModule {

class EwkContextManager : public IContextManager {
    public:
        EwkContextManager(
            const std::string& tizenAppId,
            Ewk_Context* ewkContext,
            ViewModule::IViewModulePtr viewModule);
        Ewk_Context * getEwkContext() const;
        void handleLowMemory();
        ~EwkContextManager();

    private:
        bool initialize();
        void destroy();
        void setCallbacks();
        void unsetCallbacks();
        void setW3CFeatureByPrivilege();
        void setNetworkProxy();

        // ewk context callback functions
        static void messageFromInjectedBundleCallback(
                const char* name,
                const char* body,
                char** returnData,
                void* clientInfo);
        static void didStartDownloadCallback(const char* downloadUrl, void* data);

        // vconf callback functions
        static void vconfChangedCallback(keynode_t* keynode, void* data);

        // members
        bool m_initialized;
        bool m_isInternalContext;
};
} // namespace ViewModule

#endif // EWK_CONTEXT_MANAGER_H
