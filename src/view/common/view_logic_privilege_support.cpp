/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    view_logic_privilege_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "view_logic_privilege_support.h"

#include <cwchar>
#include <map>

#include <dpl/log/log.h>
#include <dpl/foreach.h>
#include <dpl/string.h>
#include <widget_model.h>

namespace ViewModule {
namespace {
const std::map<PrivilegeSupport::Privilege, std::string> privilegeTextMap = {
    {PrivilegeSupport::Privilege::CAMERA,       "http://tizen.org/privilege/camera"},
    {PrivilegeSupport::Privilege::LOCATION,     "http://tizen.org/privilege/location"},
    {PrivilegeSupport::Privilege::MEDIACAPTURE, "http://tizen.org/privilege/mediacapture"},
    {PrivilegeSupport::Privilege::RECORDER,     "http://tizen.org/privilege/recorder"}
};
}

//Implementation class
class PrivilegeSupportImplementation
{
  private:
    WidgetModel* m_model;
    bool m_isLegacyPolicy;
    std::map<PrivilegeSupport::Privilege, DPL::OptionalBool> m_cacheResult;

    void initializePrivilegePolicy(void)
    {
        Assert(m_model);

        DPL::OptionalString requiredVersionString = m_model->RequiredVersion.Get();
        if (!requiredVersionString) {
            // tizen required_version is mandatory element
            Assert(false && "Required version is empty");
        }

        double requiredVersion = atof(DPL::ToUTF8String(*requiredVersionString).c_str());

        if (requiredVersion < 2.3) {
            // tizen v1.0 ~ v2.2
            m_isLegacyPolicy = true;
        } else {
            // tizen v2.3 ~
            m_isLegacyPolicy = false;
        }
    }

    bool isCached(PrivilegeSupport::Privilege priv)
    {
        return m_cacheResult.find(priv) != m_cacheResult.end();
    }

    void setCache(PrivilegeSupport::Privilege priv, DPL::OptionalBool result)
    {
        m_cacheResult[priv] = result;
    }

    DPL::OptionalBool getCache(PrivilegeSupport::Privilege priv)
    {
        return m_cacheResult[priv];
    }

    bool searchPrivilege(PrivilegeSupport::Privilege priv)
    {
        std::string target = privilegeTextMap.find(priv)->second;
        WrtDB::PrivilegeList list = m_model->WidgetPrivilegeList.Get();
        FOREACH(it, list) {
            if (target == DPL::ToUTF8String(*it)) {
                return true;
            }
        }
        return false;
    }


  public:
    PrivilegeSupportImplementation(WidgetModel* model) :
        m_model(model),
        m_isLegacyPolicy(false)
    {
        // Distribute required version
        initializePrivilegePolicy();
    }

    ~PrivilegeSupportImplementation()
    {
    }

    DPL::OptionalBool getPrivilegeStatus(PrivilegeSupport::Privilege priv)
    {
        // ALLOW : return true
        // DENY : return false
        // ASK : return empty
        DPL::OptionalBool ret;

        if (m_isLegacyPolicy) {
            return ret;
        }

        if (isCached(priv)) {
            return getCache(priv);
        }

        if (searchPrivilege(priv)) {
            setCache(priv, ret);
            return ret;
        }

        ret = false;
        setCache(priv, ret);
        return ret;
    }
};

PrivilegeSupport::PrivilegeSupport(WidgetModel* model) : m_impl(new PrivilegeSupportImplementation(model))
{
}

PrivilegeSupport::~PrivilegeSupport()
{
}

DPL::OptionalBool PrivilegeSupport::getPrivilegeStatus(PrivilegeSupport::Privilege priv)
{
    return m_impl->getPrivilegeStatus(priv);
}

} //namespace ViewModule
