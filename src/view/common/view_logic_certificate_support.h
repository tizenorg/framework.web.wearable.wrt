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
 * @file    view_logic_certificate_origin_support.h
 * @author  Leerang Song (leerang.song@samsung.com)
 * @version 1.0
 * @brief   Header file for certificate
 */

#ifndef VIEW_LOGIC_CERTIFICATE_SUPPORT_H_
#define VIEW_LOGIC_CERTIFICATE_SUPPORT_H_

#include <memory>
#include <Evas.h>
#include <Elementary.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <wrt-commons/certificate-dao/certificate_dao.h>

class WidgetModel;
namespace CertificateDB {
class CertificateDAO;
}

namespace ViewModule {
class CertificateSupportImplementation;

class CertificateSupport
{
  public:
    CertificateSupport(WidgetModel* widgetModel);
    virtual ~CertificateSupport();
    CertificateDB::CertificateDAO* getCertificateDAO();

  private:
    std::unique_ptr<CertificateSupportImplementation> m_impl;
};

namespace CertificateSupportUtil {
class PermissionData
{
  public:
    CertificateDB::CertificateDAO* m_certiDao;
    CertificateDB::CertificateData m_certiData;
    void* m_data;

    PermissionData(
        CertificateDB::CertificateDAO* certiDao,
        CertificateDB::CertificateData certiData,
        void* data) :
        m_certiDao(certiDao),
        m_certiData(certiData),
        m_data(data)
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
CertificateDB::Result getResult(Evas_Object* button);
};
} // namespace ViewModule

#endif // VIEW_LOGIC_CERTIFICATE_SUPPORT_H_