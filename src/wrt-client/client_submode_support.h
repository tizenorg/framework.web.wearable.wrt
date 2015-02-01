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
/*
 * @file       client_submode_support.h
 * @author     Jihoon Chung (jihoon.chung@samsung.com)
 * @version    1.0
 */

#ifndef CLIENT_SUBMODE_SUPPORT_H_
#define CLIENT_SUBMODE_SUPPORT_H_

#include <memory>
#include <Ecore_X.h>

#include <dpl/wrt-dao-ro/common_dao_types.h>

namespace ClientModule {
class SubmodeSupportImplementation;
class SubmodeSupport
{
  public:
    SubmodeSupport();
    virtual ~SubmodeSupport();
    void initialize(WrtDB::TizenAppId appId);
    void deinitialize(void);
    bool isInlineMode(void);
    bool isNeedTerminateOnSuspend(void);
    bool transientWindow(Ecore_X_Window win);

  private:
    std::unique_ptr<SubmodeSupportImplementation> m_impl;
};
} // namespace ClientModule
#endif // CLIENT_SUBMODE_SUPPORT_H_