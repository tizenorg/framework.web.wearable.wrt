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
 * @file    view_logic_web_notification_permission_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @brief   Header file of web Notification permission API
 */

#ifndef VIEW_LOGIC_WEB_NOTIFICATION_PERMISSION_SUPPORT_H_
#define VIEW_LOGIC_WEB_NOTIFICATION_PERMISSION_SUPPORT_H_

#include <Elementary.h>

//Forward declarations
namespace SecurityOriginDB {
class SecurityOriginDAO;
}

namespace ViewModule {
namespace WebNotificationPermissionSupport {
void permissionRequest(
    Evas_Object* parent,
    SecurityOriginDB::SecurityOriginDAO* securityOriginDAO,
    void* data);
} // namespace WebNotificationPermissionSupport
} // namespace ViewModule
#endif // VIEW_LOGIC_WEB_NOTIFICATION_PERMISSION_SUPPORT_H_

