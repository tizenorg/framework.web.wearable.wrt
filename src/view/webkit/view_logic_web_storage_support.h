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
 * @file    view_logic_web_storage_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#ifndef VIEW_LOGIC_WEB_STORAGE_SUPPORT_H_
#define VIEW_LOGIC_WEB_STORAGE_SUPPORT_H_

#include <memory.h>
#include <Elementary.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

namespace SecurityOriginDB {
class SecurityOriginDAO;
}

namespace ViewModule {
namespace WebStorageSupport {
typedef void (*ewkQuotaReply)(Evas_Object* ewkView, Eina_Bool allow);
bool isNeedPermissionRequest(void);
void createPermissionRequest(
    Evas_Object* window,
    SecurityOriginDB::SecurityOriginDAO* securityOriginDAO,
    Evas_Object* ewkView,
    Ewk_Security_Origin* ewkOrigin,
    ewkQuotaReply replyEAPI);

} // namespace WebStorageSupport
} // namespace ViewModule

#endif // VIEW_LOGIC_WEB_STORAGE_SUPPORT_H_
