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
 * @file    view_logic_apps_support.h
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @brief   Header file of AppsSupport class used by ViewLogic
 */

#ifndef VIEW_LOGIC_APPS_SUPPORT_H_
#define VIEW_LOGIC_APPS_SUPPORT_H_

#include <memory>
#include <string>

class WidgetModel; //Forward declaration

namespace ViewModule {
class AppsSupportImplementation; //Forward declaration

class AppsSupport
{
  public:
    AppsSupport();
    virtual ~AppsSupport();

    void initialize(WidgetModel *, unsigned);
    void deinitialize();
    void html5VideoRequest(void* event_info);
    void downloadRequest(
        const char *url,
        const char *mimeType,
        const char *userParam);

  private:
    std::unique_ptr<AppsSupportImplementation> m_impl;
};
} //namespace

#endif /* VIEW_LOGIC_APPS_SUPPORT_H_ */
