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
 * @file    view_logic_security_origin_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 * @brief   Header file for security origin
 */

#ifndef VIEW_LOGIC_SECURITY_ORIGIN_SUPPORT_H_
#define VIEW_LOGIC_SECURITY_ORIGIN_SUPPORT_H_

#include <memory>

class WidgetModel;
namespace SecurityOriginDB {
class SecurityOriginDAO;
}

namespace ViewModule {
class SecurityOriginSupportImplementation;

class SecurityOriginSupport
{
  public:
    SecurityOriginSupport(WidgetModel* widgetModel);
    virtual ~SecurityOriginSupport();
    SecurityOriginDB::SecurityOriginDAO* getSecurityOriginDAO();

  private:
    std::unique_ptr<SecurityOriginSupportImplementation> m_impl;
};

} // namespace ViewModule

#endif // VIEW_LOGIC_SECURITY_ORIGIN_SUPPORT_H_
