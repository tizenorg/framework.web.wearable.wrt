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
 * @file    view_logic_certificate_confirm_support.cpp
 * @author  Leerang Song (leerang.song@samsung.com)
 */

#include "view_logic_certificate_confirm_support.h"

#include <string>
#include <sstream>
#include <dpl/log/log.h>
#include <dpl/log/secure_log.h>
#include <dpl/availability.h>
#include <dpl/assert.h>
#include <wrt-commons/certificate-dao/certificate_dao_types.h>
#include <wrt-commons/certificate-dao/certificate_dao.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>
#include <common/view_logic_certificate_support.h>
#include <Elementary.h>
#include <widget_string.h>

namespace ViewModule {
namespace CertificateConfirmSupport {
using namespace CertificateDB;
using namespace ViewModule::CertificateSupportUtil;

namespace {

// function declare
void askUserForCertificatePermission(
    Evas_Object* window,
    PermissionData* data);
void setPermissionResult(PermissionData* permData, Result result);
static void popupCallback(void* data, Evas_Object* obj, void* eventInfo);
static void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo);

void askUserForCertificatePermission(
    Evas_Object* window,
    PermissionData* data)
{
    LogDebug("askUserForCertificatePermission called");
    Ewk_Certificate_Policy_Decision* certificatePolicyDecision =
        static_cast<Ewk_Certificate_Policy_Decision*>(data->m_data);
    Assert(certificatePolicyDecision);

    std::string msg = std::string(WRT_POP_CERTIFICATE_PERMISSION)
        + " " + ewk_certificate_policy_decision_url_get(certificatePolicyDecision);
    Evas_Object* popup = createPopup(window,
                                     msg.c_str(),
                                     WRT_BODY_REMEMBER_PREFERENCE,
                                     popupCallback,
                                     eaKeyCallback,
                                     data);

    if (popup == NULL) {
        LogError("Fail to create popup object");
        delete data;
        return;
    } else {
        evas_object_show(popup);
    }
}

void setPermissionResult(PermissionData* permData, Result result)
{
    Assert(permData);
    Ewk_Certificate_Policy_Decision* certificatePolicyDecision =
        static_cast<Ewk_Certificate_Policy_Decision*>(permData->m_data);

    if (result != RESULT_UNKNOWN) {
        permData->m_certiDao->setCertificateData(permData->m_certiData, result);
    }

    Eina_Bool ret = (result == RESULT_ALLOW_ALWAYS || result == RESULT_ALLOW_ONCE) ? EINA_TRUE : EINA_FALSE;
    ewk_certificate_policy_decision_allowed_set(certificatePolicyDecision, ret);
}

void popupCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, getResult(obj));
    delete permData;

    Evas_Object* popup = getPopup(obj);
    evas_object_hide(popup);
    evas_object_del(popup);
}

void eaKeyCallback(void* data, Evas_Object* obj, void* eventInfo)
{
    _D("called");

    Assert(data);
    Assert(obj);

    DPL_UNUSED_PARAM(eventInfo);

    PermissionData* permData = static_cast<PermissionData*>(data);
    setPermissionResult(permData, RESULT_DENY_ONCE);
    delete permData;

    evas_object_hide(obj);
    evas_object_del(obj);
}
} // namespace

void certificatePermissionRequest(
    Evas_Object* window,
    CertificateDB::CertificateDAO* certificateDAO,
    void* data)
{
    LogDebug("certificationPermissionRequest called");
    Assert(certificateDAO);
    Assert(data);

    Ewk_Certificate_Policy_Decision* certificatePolicyDecision =
        static_cast<Ewk_Certificate_Policy_Decision*>(data);
    ewk_certificate_policy_decision_suspend(certificatePolicyDecision);
    Assert(certificatePolicyDecision);

    CertificateData certificateData(
             DPL::FromUTF8String(
                ewk_certificate_policy_decision_certificate_pem_get(
                     certificatePolicyDecision)));

    // check cache database
    Result result = certificateDAO->getResult(certificateData);

    if (RESULT_ALLOW_ONCE == result || RESULT_ALLOW_ALWAYS == result) {
        LogDebug("allow");
        ewk_certificate_policy_decision_allowed_set(
            certificatePolicyDecision,
            EINA_TRUE);
         return;
   } else if (RESULT_DENY_ONCE == result || RESULT_DENY_ALWAYS == result) {
        LogDebug("Deny");
        ewk_certificate_policy_decision_allowed_set(
            certificatePolicyDecision,
            EINA_FALSE);
         return;
    }
    // ask to user
    PermissionData* permissionData =
        new PermissionData(certificateDAO,
                           certificateData,
                           certificatePolicyDecision);
    askUserForCertificatePermission(window, permissionData);
    return;
}
}
} // namespace ViewModule
