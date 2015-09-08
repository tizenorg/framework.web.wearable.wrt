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
 * @file    view_logic_security_origin_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 * @brief   Support security origin dao
 */

#include "view_logic_security_origin_support.h"

#include <memory>
#include <dpl/assert.h>
#include <dpl/log/secure_log.h>
#include <wrt-commons/security-origin-dao/security_origin_dao.h>
#include <widget_model.h>

namespace ViewModule {

    class SecurityOriginSupportImplementation
{
  private:
    WidgetModel* m_model;
    SecurityOriginDB::SecurityOriginDAOPtr m_securityOriginDAO;

  public:
    SecurityOriginSupportImplementation(WidgetModel* widgetModel) :
        m_model(NULL)
    {
        Assert(widgetModel);
        m_model = widgetModel;
    }

    ~SecurityOriginSupportImplementation()
    {}

    SecurityOriginDB::SecurityOriginDAO* getSecurityOriginDAO(void)
    {
        Assert(m_model);
        if (!m_securityOriginDAO) {
            _D("initialize securityOriginDAO");
            m_securityOriginDAO =
                SecurityOriginDB::SecurityOriginDAOPtr(
                    new SecurityOriginDB::SecurityOriginDAO(m_model->TzPkgId.
                                                                Get()));
            // initialize security result data. Remove allow, deny for
            m_securityOriginDAO->removeSecurityOriginData(
                SecurityOriginDB::RESULT_ALLOW_ONCE);
            m_securityOriginDAO->removeSecurityOriginData(
                SecurityOriginDB::RESULT_DENY_ONCE);
        }
        return m_securityOriginDAO.get();
    }
};

SecurityOriginSupport::SecurityOriginSupport(WidgetModel* widgetModel) :
    m_impl(new SecurityOriginSupportImplementation(widgetModel))
{}

SecurityOriginSupport::~SecurityOriginSupport()
{}

SecurityOriginDB::SecurityOriginDAO* SecurityOriginSupport::
    getSecurityOriginDAO(void)
{
    return m_impl->getSecurityOriginDAO();
}

} // namespace ViewModule
