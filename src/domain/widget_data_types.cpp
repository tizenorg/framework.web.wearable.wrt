/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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
 * This file  have been implemented in compliance with  W3C WARP SPEC.
 * but there are some patent issue between  W3C WARP SPEC and APPLE.
 * so if you want to use this file, refer to the README file in root directory
 */
/**
 * @file       widget_data_types.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @author     Tomasz Iwanek (t.iwanek@samsung.com) (implementation moved to
 * cpp)
 * @version    0.1
 * @brief
 */

#include <widget_data_types.h>

#include <stdlib.h>

#include <dpl/foreach.h>
#include <dpl/log/secure_log.h>

namespace {
const char* const WRT_WIDGET_DATA_TYPES_LOG_ENABLE =
    "WRT_WIDGET_DATA_TYPES_LOG_ENABLE";
}

WidgetAccessList::WidgetAccessList() : m_isAccessAll(false)
{}

WidgetAccessList::WidgetAccessList(
    const WrtDB::WidgetAccessInfoList &widgetAccessInfoList) :
    m_isAccessAll(false)
{
    FOREACH(it, widgetAccessInfoList)
    {
        if (DPL::FromUTF32String(L"*") == it->strIRI) {
            m_isAccessAll = true;
            m_warpIriList.clear();
            return;
        }

        WarpIRI warpIri;

        warpIri.set(DPL::ToUTF8String(it->strIRI).c_str(),
                    it->bSubDomains);

        if (warpIri.isAccessDefinition()) {
            m_warpIriList.push_back(warpIri);
        }
    }
}

bool WidgetAccessList::getIsAccessAll() const
{
    return m_isAccessAll;
}

const WarpIRIList* WidgetAccessList::getWarpIRIList() const
{
    return &m_warpIriList;
}

bool WidgetAccessList::isRequiredIRI(const DPL::String &iri) const
{
    if (m_isAccessAll) {
        return true;
    }

    WarpIRI requestIri;
    requestIri.set(iri.c_str(), false);

    FOREACH(it, m_warpIriList)
    {
        if (it->isSubDomain(requestIri)) {
            return true;
        }
    }

    return false;
}

bool WidgetAccessList::operator ==(const WidgetAccessList& other) const
{
    return m_warpIriList == other.m_warpIriList &&
           m_isAccessAll == other.m_isAccessAll;
}

WidgetSettingList::WidgetSettingList() :
    m_RotationLock(Screen_Portrait),
    m_IndicatorPresence(Indicator_Enable),
    m_BackButtonPresence(BackButton_Disable),
    m_ContextMenu(ContextMenu_Enable),
    m_Encryption(Encryption_Disable),
    m_BackgroundSupport(BackgroundSupport_Disable),
    m_ProgressbarPresence(ProgressBar_Disable),
    m_HWkeyEvent(HWkeyEvent_Enable),
    m_Accessibility(Accessibility_Enable),
    m_SoundMode(SoundMode_Shared),
    m_BackgroundVibration(BackgroundVibration_Disable)
{
    m_logEnable = (getenv(WRT_WIDGET_DATA_TYPES_LOG_ENABLE) != NULL);
}

WidgetSettingList::WidgetSettingList(WrtDB::WidgetSettings &widgetSettings)
{
    using namespace TizenSetting::Name;
    using namespace TizenSetting::Value;

    m_logEnable = (getenv(WRT_WIDGET_DATA_TYPES_LOG_ENABLE) != NULL);

    m_RotationLock = Screen_Portrait;
    m_IndicatorPresence = Indicator_Enable;
    m_BackButtonPresence = BackButton_Disable;
    m_ContextMenu = ContextMenu_Enable;
    m_Encryption = Encryption_Disable;
    m_BackgroundSupport = BackgroundSupport_Disable;
    m_ProgressbarPresence = ProgressBar_Disable;
    m_HWkeyEvent = HWkeyEvent_Enable;
    m_Accessibility = Accessibility_Enable;
    m_SoundMode = SoundMode_Shared;
    m_BackgroundVibration = BackgroundVibration_Disable;

    FOREACH(it, widgetSettings) {
        DPL::String name = it->settingName;
        DPL::String value = it->settingValue;

        if (name == SCREEN_ORIENTATION) {
            if (value == SCREEN_ORIENTATION_PORTRAIT) {
                m_RotationLock = Screen_Portrait;
            } else if (value == SCREEN_ORIENTATION_LANDSCAPE) {
                m_RotationLock = Screen_Landscape;
            } else if (value == SCREEN_ORIENTATION_AUTO_ROTATION) {
                m_RotationLock = Screen_AutoRotation;
            } else {
                displayError(name, value);
                m_RotationLock = Screen_Portrait;
            }
        } else if (name == INDICATOR_PRESENCE) {
            if (value == INDICATOR_PRESENCE_ENALBE) {
                m_IndicatorPresence = Indicator_Enable;
            } else if (value == INDICATOR_PRESENCE_DISABLE) {
                m_IndicatorPresence = Indicator_Disable;
            } else {
                displayError(name, value);
                m_IndicatorPresence = Indicator_Enable;
            }
        } else if (name == BACKBUTTON_PRESENCE) {
            if (value == BACKBUTTON_PRESENCE_ENALBE) {
                m_BackButtonPresence = BackButton_Enable;
            } else if (value == BACKBUTTON_PRESENCE_DISABLE) {
                m_BackButtonPresence = BackButton_Disable;
            } else {
                displayError(name, value);
                m_BackButtonPresence = BackButton_Disable;
            }
        } else if (name == CONTEXT_MENU) {
            if (value == CONTEXT_MENU_ENABLE) {
                m_ContextMenu = ContextMenu_Enable;
            } else if (value == CONTEXT_MENU_DISABLE) {
                m_ContextMenu = ContextMenu_Disable;
            } else {
                displayError(name, value);
                m_ContextMenu = ContextMenu_Enable;
            }
        } else if (name == ENCRYPTION) {
            if (value == ENCRYPTION_ENABLE) {
                m_Encryption = Encryption_Enable;
            } else if (value == ENCRYPTION_DISABLE) {
                m_Encryption = Encryption_Disable;
            } else {
                displayError(name, value);
                m_Encryption = Encryption_Disable;
            }
        } else if (name == BACKGROUND_SUPPORT) {
            if (value == ENABLE) {
                m_BackgroundSupport = BackgroundSupport_Enable;
            } else if (value == DISABLE) {
                m_BackgroundSupport = BackgroundSupport_Disable;
            } else {
                displayError(name, value);
                m_BackgroundSupport = BackgroundSupport_Disable;
            }
        }
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
        else if (name == USER_AGENT) {
            DPL::OptionalString userAgent = value;
            if (!!userAgent) {
                m_UserAgent = DPL::ToUTF8String(*userAgent);
            }
        }
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
        else if (name == PROGRESSBAR_PRESENCE) {
            if (value == PROGRESSBAR_PRESENCE_ENABLE) {
                m_ProgressbarPresence = ProgressBar_Enable;
            } else if (value == PROGRESSBAR_PRESENCE_DISABLE) {
                m_ProgressbarPresence = ProgressBar_Disable;
            } else {
                displayError(name, value);
                m_ProgressbarPresence = ProgressBar_Disable;
            }
        } else if (name == HWKEY_EVENT) {
            if (value == HWKEY_EVENT_ENABLE) {
                m_HWkeyEvent = HWkeyEvent_Enable;
            } else if (value == HWKEY_EVENT_DISABLE) {
                m_HWkeyEvent = HWkeyEvent_Disable;
            } else {
                displayError(name, value);
                m_HWkeyEvent = HWkeyEvent_Enable;
            }
        } else if (name == ACCESSIBILITY) {
            if (value == ACCESSIBILITY_ENABLE) {
                m_Accessibility = Accessibility_Enable;
            } else if (value == ACCESSIBILITY_DISABLE) {
                m_Accessibility = Accessibility_Disable;
            } else {
                displayError(name, value);
                m_Accessibility = Accessibility_Enable;
            }
        } else if (name == SOUND_MODE) {
            if (value == SOUND_MODE_SAHRED) {
                m_SoundMode = SoundMode_Shared;
            } else if (value == SOUND_MODE_EXCLUSIVE) {
                m_SoundMode = SoundMode_Exclusive;
            } else {
                displayError(name, value);
                m_SoundMode = SoundMode_Shared;
            }
        } else if (name == BACKGROUND_VIBRATION) {
            if (value == ENABLE) {
                m_BackgroundVibration = BackgroundVibration_Enable;
            } else if (value == DISABLE) {
                m_BackgroundVibration = BackgroundVibration_Disable;
            } else {
                displayError(name, value);
                m_BackgroundVibration = BackgroundVibration_Disable;
            }
        } else {
            displayError(name, value);
        }
    }
}

WidgetSettingScreenLock WidgetSettingList::getRotationValue() const
{
    if (m_logEnable) {_D("m_RotationLock: %d", m_RotationLock);}
    return m_RotationLock;
}

WidgetSettingIndicatorPresence WidgetSettingList::getIndicatorPresence() const
{
    if (m_logEnable) {_D("m_IndicatorPresence: %d", m_IndicatorPresence);}
    return m_IndicatorPresence;
}

WidgetSettingBackButtonPresence WidgetSettingList::getBackButtonPresence()
const
{
    if (m_logEnable) {_D("m_BackButtonPresence: %d", m_BackButtonPresence);}
    return m_BackButtonPresence;
}

WidgetSettingContextMenu WidgetSettingList::getContextMenu() const
{
    if (m_logEnable) {_D("m_ContextMenu: %d", m_ContextMenu);}
    return m_ContextMenu;
}

WidgetSettingEncryption WidgetSettingList::getEncryption() const
{
    if (m_logEnable) {_D("m_Encryption: %d", m_Encryption);}
    return m_Encryption;
}

WidgetSettingBackgroundSupport WidgetSettingList::getBackgroundSupport() const
{
    if (m_logEnable) {_D("m_BackgroundSupport: %d", m_BackgroundSupport);}
    return m_BackgroundSupport;
}

#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
std::string WidgetSettingList::getUserAgent() const
{
    if (m_logEnable) {_D("m_UserAgent: %s", m_UserAgent.c_str());}
    return m_UserAgent;
}
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)

WidgetSettingProgressBarPresence WidgetSettingList::getProgressBarPresence() const
{
    if (m_logEnable) {_D("m_ProgressbarPresence: %d", m_ProgressbarPresence);}
    return m_ProgressbarPresence;
}

WidgetSettingHWkeyEventPresence WidgetSettingList::getHWkeyEvent() const
{
    if (m_logEnable) {_D("m_HWkeyEvent: %d", m_HWkeyEvent);}
    return m_HWkeyEvent;
}

WidgetSettingAccessibility WidgetSettingList::getAccessibility() const
{
    if (m_logEnable) {_D("m_Accessibility: %d", m_Accessibility);}
    return m_Accessibility;
}

WidgetSettingSoundMode WidgetSettingList::getSoundMode() const
{
    if (m_logEnable) {_D("m_SoundMode: %d", m_SoundMode);}
    return m_SoundMode;
}

WidgetSettingBackgroundVibration WidgetSettingList::getBackgroundVibration() const
{
    if (m_logEnable) {_D("m_BackgroundVibration: %d", m_BackgroundVibration);}
    return m_BackgroundVibration;
}

bool WidgetSettingList::operator ==(const WidgetSettingList& other) const
{
    return m_RotationLock == other.m_RotationLock &&
           m_IndicatorPresence == other.m_IndicatorPresence &&
           m_BackButtonPresence == other.m_BackButtonPresence &&
           m_ContextMenu == other.m_ContextMenu &&
           m_Encryption == other.m_Encryption &&
           m_BackgroundSupport == other.m_BackgroundSupport &&
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
           m_UserAgent == other.m_UserAgent &&
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
           m_ProgressbarPresence == other.m_ProgressbarPresence &&
           m_HWkeyEvent == other.m_HWkeyEvent &&
           m_Accessibility == other.m_Accessibility &&
           m_SoundMode == other.m_SoundMode &&
           m_BackgroundVibration== other.m_BackgroundVibration;
}

void WidgetSettingList::displayError(const DPL::String& name,
                                     const DPL::String& value)
{
    _W("Invalid \"%s\" setting value \"%s\"",
       DPL::ToUTF8String(name).c_str(),
       DPL::ToUTF8String(value).c_str());
}
