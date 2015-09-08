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
 * @file    application_data.cpp
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @version 1.0
 * @brief   implementation file for application_data.h
 */

#include "application_data.h"
#include <dpl/singleton_safe_impl.h>
#include <dpl/log/log.h>
#include <bundle.h>

IMPLEMENT_SAFE_SINGLETON(ApplicationData)

ApplicationData::ApplicationData() :
    m_originBundle(NULL),
    m_encodedBundle(NULL)
{}

ApplicationData::~ApplicationData()
{}

bundle* ApplicationData::getBundle() const
{
    return m_originBundle;
}

const char* ApplicationData::getEncodedBundle() const
{
    return (const char *)m_encodedBundle;
}

bool ApplicationData::setBundle(bundle *originBundle)
{
    if (!originBundle) {
        LogError("Bundle is empty!");
        return false;
    }

    freeBundle();

    m_originBundle = originBundle;
    return true;
}

bool ApplicationData::setEncodedBundle(bundle* originBundle)
{
    int len, ret;
    if (!originBundle) {
        LogError("Bundle is empty!");
        return false;
    }

    freeEncodedBundle();

    ret = bundle_encode(originBundle, &m_encodedBundle, &len);
    if (ret == -1) {
        LogError("Failed to encode bundle data");
        return false;
    }

    LogDebug("Encoded Bundle : " << m_encodedBundle);
    return true;
}

void ApplicationData::freeBundle()
{
    if (!m_originBundle) {
        return;
    }

    if (!bundle_free(m_originBundle)) {
        LogDebug("Bundle data freed for new bundle data");
        m_originBundle = NULL;
    }
}

void ApplicationData::freeEncodedBundle()
{
    if (!m_encodedBundle) {
        return;
    }

    if (m_encodedBundle) {
        if (!bundle_free_encoded_rawdata(
                &m_encodedBundle))
        {
            LogDebug("Bundle data freed for new bundle data");
            m_encodedBundle = NULL;
        }
    }
}

