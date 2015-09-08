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
 * @file    view_logic_security_origin_support_util.h
 * @author  Adam Banasiak (a.banasiak@samsung.com)
 * @version 1.0
 * @brief   Header file for security origin utility
 */

#ifndef VIEW_LOGIC_SECURITY_ORIGIN_SUPPORT_UTILITY_H
#define VIEW_LOGIC_SECURITY_ORIGIN_SUPPORT_UTILITY_H

#include <Evas.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <dpl/string.h>

namespace ViewModule {
namespace SecurityOriginSupportUtil {
class PermissionData
{
  public:
    SecurityOriginDB::SecurityOriginDAO* m_originDao;
    SecurityOriginDB::SecurityOriginData m_originData;
    void* m_data;
    DPL::String m_pkgId;

    PermissionData(
        SecurityOriginDB::SecurityOriginDAO* originDao,
        SecurityOriginDB::SecurityOriginData originData,
        void* data) :
        m_originDao(originDao),
        m_originData(originData),
        m_data(data)
    {}

    PermissionData(
        SecurityOriginDB::SecurityOriginDAO* originDao,
        SecurityOriginDB::SecurityOriginData originData,
        void* data,
        DPL::String& pkgId):
        m_originDao(originDao),
        m_originData(originData),
        m_data(data),
        m_pkgId(pkgId)
    {}
};

Evas_Object* createPopup(Evas_Object* window,
                         const char* bodyText,
                         const char* checkText,
                         Evas_Smart_Cb buttonCallback,
                         Evas_Smart_Cb keyCallback,
                         void* data);
Evas_Object* getPopup(Evas_Object* button);
Evas_Object* getCheck(Evas_Object* popup);
SecurityOriginDB::Result getResult(Evas_Object* button);
bool isNeedHelpPopup(Evas_Object* popup);
}  // namespace SecurityOriginSupportUtil
}  // namespace ViewModule


#endif /* VIEW_LOGIC_SECURITY_ORIGIN_SUPPORT_UTILITY_H */

