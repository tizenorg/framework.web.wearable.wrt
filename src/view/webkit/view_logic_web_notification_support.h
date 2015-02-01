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
 * @file    view_logic_web_notification_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @brief   Header file of web Notification API used by ViewLogic
 */

#ifndef VIEW_LOGIC_WEB_NOTIFICATION_SUPPORT_H_
#define VIEW_LOGIC_WEB_NOTIFICATION_SUPPORT_H_

#include <memory>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include "view_logic_web_notification_data.h"

namespace ViewModule {
class WebNotificationSupportImplementation;
class WebNotificationSupport
{
  public:
    WebNotificationSupport();
    virtual ~WebNotificationSupport();
    void initialize(WrtDB::TizenPkgId pkgId);
    void deinitialize(void);

    bool show(WebNotificationDataPtr notiData);
    void* hide(uint64_t ewkNotiId);

  private:
    std::unique_ptr<WebNotificationSupportImplementation> m_impl;
};
} // namespace ViewModule
#endif /* VIEW_LOGIC_WEB_NOTIFICATION_H_ */
