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
 * @file    view_logic_certificate_confirm_support.h
 * @author  Leerang Song (leerang.song@samsung.com)
 */

#ifndef VIEW_LOGIC_CERTIFICATE_CONFIRM_SUPPORT_H_
#define VIEW_LOGIC_CERTIFICATE_CONFIRM_SUPPORT_H_

#include <memory.h>
#include <Elementary.h>

namespace CertificateDB {
class CertificateDAO;
}

namespace ViewModule {
namespace CertificateConfirmSupport {
void certificatePermissionRequest(
    Evas_Object* window,
    CertificateDB::CertificateDAO* certificateDAO,
    void* data);
}
} // namespace ViewModule

#endif
