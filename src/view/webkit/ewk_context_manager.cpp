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
 * @file    ewk_context_manager.cpp
 * @author  Yunchan Cho (yunchan.cho@samsung.com)
 * @version 0.1
 * @brief   Implementation of EwkContextManager class.
 *          This file handles operation regarding Ewk_Context
 */

#include "ewk_context_manager.h"

#include <map>
#include <string>

#include <cert-service.h>
#include <vconf.h>
#include <dpl/assert.h>
#include <dpl/log/log.h>
#include <dpl/foreach.h>
#include <dpl/platform.h>
#include <dpl/wrt-dao-ro/widget_config.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/widget_dao_types.h>
#include <EWebKit.h>
#include <EWebKit_internal.h>

#include <i_view_module.h>
#include <profiling_util.h>
#include <widget_data_types.h>

namespace ViewModule {
namespace {
const std::string bundlePath("/usr/lib/libwrt-injected-bundle.so");
std::map<Ewk_Extensible_API, Eina_Bool> defaultExtensibleAPI = {
    { EWK_EXTENSIBLE_API_MEDIA_STREAM_RECORD,           EINA_TRUE  },
    { EWK_EXTENSIBLE_API_ROTATE_CAMERA_VIEW,            EINA_FALSE },
#if !USE(WEB_PROVIDER_EXCEPTION_IN_EWK_CONTEXT)
    { EWK_EXTENSIBLE_API_MEDIA_VOLUME_CONTROL,          EINA_TRUE  },
#endif
    { EWK_EXTENSIBLE_API_FULL_SCREEN_FORBID_AUTO_EXIT,  EINA_TRUE  },
    { EWK_EXTENSIBLE_API_ENCRYPTION_DATABASE,           EINA_TRUE  },
    { EWK_EXTENSIBLE_API_XWINDOW_FOR_FULL_SCREEN_VIDEO, EINA_TRUE  },
    { EWK_EXTENSIBLE_API_CAMERA_CONTROL,                EINA_FALSE }
};
} // anonymous namespace

EwkContextManager::EwkContextManager(
        const std::string& tizenAppId,
        Ewk_Context* ewkContext,
        IViewModulePtr viewModule)
    : IContextManager(tizenAppId, ewkContext, viewModule),
      m_initialized(false), m_isInternalContext(false)
{
    if (!initialize()) {
        ThrowMsg(DPL::Exception, "Fail to intialize EwkContextManager");
    }
    // set ewk context callbacks
    setCallbacks();
}

EwkContextManager::~EwkContextManager()
{
    // unset registered ewk context callbacks
    unsetCallbacks();
    destroy();
}

Ewk_Context * EwkContextManager::getEwkContext() const
{
    return m_ewkContext;
}

bool EwkContextManager::initialize()
{
    if (!m_ewkContext) {
        m_ewkContext = ewk_context_new_with_injected_bundle_path(bundlePath.c_str());

        if (!m_ewkContext) {
            return false;
        }

        m_isInternalContext = true;
    }

    // cache model setting
    ewk_context_cache_model_set(
        m_ewkContext,
        EWK_CACHE_MODEL_DOCUMENT_BROWSER);
    ADD_PROFILING_POINT("WebProcess fork", "start");
    // To fork a Webprocess as soon as possible,
    // the following ewk_api is called explicitly.
    ewk_cookie_manager_accept_policy_set(
            ewk_context_cookie_manager_get(m_ewkContext),
            EWK_COOKIE_ACCEPT_POLICY_ALWAYS);
    ADD_PROFILING_POINT("WebProcess fork", "stop");

    // proxy server setting
    setNetworkProxy();

    LogDebug("ewk_context_certificate_file_set() was called.");
    const char* caCertPath = cert_svc_get_certificate_crt_file_path();
    if (caCertPath) {
        ewk_context_certificate_file_set(m_ewkContext, caCertPath);
    } else {
        LogError("cert path is null");
    }

    FOREACH(it, defaultExtensibleAPI) {
        ewk_context_tizen_extensible_api_set(m_ewkContext,
                                             it->first,
                                             it->second);
    }

    // ewk storage_path set
    ewk_context_storage_path_reset(m_ewkContext);

    // set w3c features
    setW3CFeatureByPrivilege();

    // web application dependent settings
    WrtDB::WidgetDAOReadOnly dao(DPL::FromUTF8String(m_appId));
#if ENABLE(CONTENT_SECURITY_POLICY)
    if (dao.getSecurityModelVersion() ==
        WrtDB::WidgetSecurityModelVersion::WIDGET_SECURITY_MODEL_V2)
    {
        ewk_context_tizen_extensible_api_set(m_ewkContext,
                                             EWK_EXTENSIBLE_API_CSP,
                                             EINA_TRUE);
    }
#endif

    // WidgetSettingList dependent settings
    WrtDB::WidgetSettings widgetSettings;
    dao.getWidgetSettings(widgetSettings);
    WidgetSettingList settings(widgetSettings);

    ewk_context_tizen_extensible_api_set(
        m_ewkContext,
        EWK_EXTENSIBLE_API_BACKGROUND_MUSIC,
        settings.getBackgroundSupport() == BackgroundSupport_Enable ?
            EINA_TRUE : EINA_FALSE);
    ewk_context_tizen_extensible_api_set(
        m_ewkContext,
        EWK_EXTENSIBLE_API_SOUND_MODE,
        settings.getSoundMode() == SoundMode_Exclusive ?
            EINA_TRUE : EINA_FALSE);
    ewk_context_tizen_extensible_api_set(
        m_ewkContext,
        EWK_EXTENSIBLE_API_ROTATION_LOCK,
        settings.getRotationValue() == Screen_AutoRotation ?
            EINA_FALSE : EINA_TRUE);

    // note: EWK_EXTENSIBLE_API_VISIBILITY_SUSPEND - private package only
    ewk_context_tizen_extensible_api_set(
        m_ewkContext,
        EWK_EXTENSIBLE_API_VISIBILITY_SUSPEND,
        settings.getBackgroundSupport() == BackgroundSupport_Enable ?
            EINA_TRUE : EINA_FALSE);

    ewk_context_tizen_extensible_api_set(
        m_ewkContext,
        EWK_EXTENSIBLE_API_BACKGROUND_VIBRATION,
        settings.getBackgroundVibration() == BackgroundVibration_Enable ?
            EINA_TRUE : EINA_FALSE);

    std::string pluginsPath =
        WrtDB::WidgetConfig::GetWidgetNPRuntimePluginsPath(
            dao.getTizenPkgId());

    // npruntime plugins path set
    LogDebug("ewk_context_additional_plugin_path_set() : " << pluginsPath);

    ewk_context_additional_plugin_path_set(m_ewkContext, pluginsPath.c_str());

    m_initialized = true;

    return true;
}

void EwkContextManager::destroy()
{
    // only in the following case, webkit context should be deleted
    if (m_initialized && m_isInternalContext) {
        ewk_object_unref(m_ewkContext);
    }
}

void EwkContextManager::setCallbacks()
{
    if (!m_initialized) {
        return;
    }

    ewk_context_message_from_injected_bundle_callback_set(
            m_ewkContext,
            messageFromInjectedBundleCallback,
            this);

    ewk_context_did_start_download_callback_set(
            m_ewkContext,
            didStartDownloadCallback,
            this);

    vconf_notify_key_changed(VCONFKEY_NETWORK_PROXY,
                             vconfChangedCallback,
                             this);
}

void EwkContextManager::unsetCallbacks()
{
    if (!m_initialized) {
        return;
    }

    ewk_context_message_from_injected_bundle_callback_set(
            m_ewkContext, NULL, NULL);
    ewk_context_did_start_download_callback_set(
            m_ewkContext, NULL, NULL);

    vconf_ignore_key_changed(VCONFKEY_NETWORK_PROXY,
                             vconfChangedCallback);
}

void EwkContextManager::setW3CFeatureByPrivilege()
{
    using namespace WrtDB;
    WrtDB::WidgetDAOReadOnly dao(DPL::FromUTF8String(m_appId));
    if(!m_ewkContext) {
        return;
    }
    std::list<DPL::String> widgetPrivilege = dao.getWidgetPrivilege();

    bool camera = false;
    bool fullscreen = false;
    bool audio = false;

    FOREACH(it, widgetPrivilege) {
        std::map<std::string, Feature>::const_iterator result =
        g_W3CPrivilegeTextMap.find(DPL::ToUTF8String(*it));
        if (result != g_W3CPrivilegeTextMap.end()) {
            switch (result->second) {
            case FEATURE_FULLSCREEN_MODE:
                LogDebug("fullscreen feature exists");
                fullscreen = true;
                break;
            case FEATURE_CAMERA:
                LogDebug("camera feature exists");
                camera = true;
                break;
           case FEATURE_AUDIO_RECORDER:
                LogDebug("audiorecorder feature exists");
                audio = true;
                break;
            default:
                break;
            }
        }
    }
    // set auto full screen mode
    ewk_context_tizen_extensible_api_set(m_ewkContext,
        EWK_EXTENSIBLE_API_FULL_SCREEN,
        fullscreen);

    // set camera
    ewk_context_tizen_extensible_api_set(m_ewkContext,
        EWK_EXTENSIBLE_API_CAMERA_CONTROL,
        camera);

    // set audio recorder
    ewk_context_tizen_extensible_api_set(m_ewkContext,
        EWK_EXTENSIBLE_API_AUDIO_RECORDING_CONTROL,
        audio);
}

void EwkContextManager::setNetworkProxy()
{
    Assert(m_ewkContext);

    char* proxy = vconf_get_str(VCONFKEY_NETWORK_PROXY);
    if (proxy && strlen(proxy) && strcmp(proxy, "0.0.0.0")) {
        LogDebug("proxy address: " << proxy);
        ewk_context_proxy_uri_set(m_ewkContext, proxy);
    } else {
        LogDebug("proxy address is empty");
        ewk_context_proxy_uri_set(m_ewkContext, NULL);
    }
}

void EwkContextManager::messageFromInjectedBundleCallback(
        const char* name,
        const char* body,
        char** returnData,
        void* clientInfo)
{
    LogDebug("enter");

    EwkContextManager* This = static_cast<EwkContextManager*>(clientInfo);
    if (returnData) {
        This->m_view->checkSyncMessageFromBundle(name, body, returnData);
    } else {
        This->m_view->checkAsyncMessageFromBundle(name, body);
    }
}

void EwkContextManager::didStartDownloadCallback(const char* downloadUrl, void* data)
{
    LogDebug("enter");

    EwkContextManager* This = static_cast<EwkContextManager*>(data);
    This->m_view->downloadData(downloadUrl);
}

void EwkContextManager::vconfChangedCallback(keynode_t* keynode, void* data)
{
    LogDebug("enter");
    char* key = vconf_keynode_get_name(keynode);

    if (NULL == key) {
        LogError("key is null");
        return;
    }
    std::string keyString = key;
    Assert(data);
    EwkContextManager* This = static_cast<EwkContextManager*>(data);
    if (keyString == VCONFKEY_NETWORK_PROXY) {
        This->setNetworkProxy();
    }
}

void EwkContextManager::handleLowMemory()
{
    if (!m_ewkContext) {
        return;
    }

    //ewk_context_resource_cache_clear(m_ewkContext);
    //ewk_context_notify_low_memory(m_ewkContext);
}

ContextManagerPtr createEwkContextManager(
        std::string& tizenAppId,
        Ewk_Context* ewkContext,
        ViewModule::IViewModulePtr viewModule)
{
    return ContextManagerPtr(new EwkContextManager(tizenAppId, ewkContext, viewModule));
}

} // namespace ViewModule
