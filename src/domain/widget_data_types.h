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
 * This file  have been implemented in compliance with  W3C WARP SPEC.
 * but there are some patent issue between  W3C WARP SPEC and APPLE.
 * so if you want to use this file, refer to the README file in root directory
 */
/**
 * @file       widget_data_types.h
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    0.1
 * @brief
 */

#ifndef WRT_SRC_DOMAIN_WIDGET_DATA_TYPES_H_
#define WRT_SRC_DOMAIN_WIDGET_DATA_TYPES_H_

#include <list>
#include <memory>

#include <dpl/platform.h>
#include <dpl/utils/warp_iri.h>
#include <dpl/utils/widget_version.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

// WidgetIcon, LanguageTagsList, OptionalWidgetStartFileInfo,
// WidgetStartFileInfo
#include <dpl/localization/localization_utils.h>

typedef std::list<WarpIRI> WarpIRIList;

/**
 * @brief Execution phase according to BONDI
 *
 * WidgetExecutionPhase_Unknown:
 *  Execution state is not defined
 *
 * WidgetExecutionPhase_WidgetInstall:
 *  Applies to access control queries made by a Widget User Agent during the
 *  processing of a Widget Resource as part of an installation or update
 *  operation
 *
 * WidgetExecutionPhase_WidgetInstantiate:
 *  Applies to access control queries made by a Widget User Agent during the
 *  instantiation of a Widget
 *
 * WidgetExecutionPhase_WebkitBind:
 *  Applies to access control queries made in response to a call to
 *  requestFeature() in the course of execution of a Website
 *
 * WidgetExecutionPhase_Invoke:
 *  Applies to access control queries made in response to invocation of a
 *  JavaScript API in the course of execution of a Web Application
 */
//enum WidgetExecutionPhase
//{
//    WidgetExecutionPhase_Unknown           = 0,
//    WidgetExecutionPhase_WidgetInstall     = 1 << 0,
//    WidgetExecutionPhase_WidgetInstantiate = 1 << 1,
//    WidgetExecutionPhase_WebkitBind        = 1 << 2,
//    WidgetExecutionPhase_Invoke            = 1 << 3
//};

class WidgetModel;

class WidgetAccessList
{
  public:
    WidgetAccessList();

    WidgetAccessList(const WrtDB::WidgetAccessInfoList &widgetAccessInfoList);

    bool getIsAccessAll() const;

    const WarpIRIList* getWarpIRIList() const;

    bool isRequiredIRI(const DPL::String &iri) const;

    bool operator ==(const WidgetAccessList& other) const;

  private:
    WarpIRIList m_warpIriList;
    bool m_isAccessAll;
};

namespace TizenSetting {
namespace Name {
const DPL::String SCREEN_ORIENTATION   = L"screen-orientation";
const DPL::String INDICATOR_PRESENCE   = L"indicator-presence";
const DPL::String BACKBUTTON_PRESENCE  = L"backbutton-presence";
const DPL::String CONTEXT_MENU         = L"context-menu";
const DPL::String BACKGROUND_SUPPORT   = L"background-support";
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
const DPL::String USER_AGENT           = L"user-agent";
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
const DPL::String PROGRESSBAR_PRESENCE = L"progressbar-presence";
const DPL::String HWKEY_EVENT          = L"hwkey-event";
const DPL::String ACCESSIBILITY        = L"accessibility";
const DPL::String SOUND_MODE           = L"sound-mode";
const DPL::String ENCRYPTION           = L"encryption";
const DPL::String BACKGROUND_VIBRATION = L"background-vibration";
} // namespace Name
namespace Value {
const DPL::String ENABLE                           = L"enable";
const DPL::String DISABLE                          = L"disable";
const DPL::String SCREEN_ORIENTATION_PORTRAIT      = L"portrait";
const DPL::String SCREEN_ORIENTATION_LANDSCAPE     = L"landscape";
const DPL::String SCREEN_ORIENTATION_AUTO_ROTATION = L"auto-rotation";
const DPL::String INDICATOR_PRESENCE_ENALBE        = ENABLE;
const DPL::String INDICATOR_PRESENCE_DISABLE       = DISABLE;
const DPL::String BACKBUTTON_PRESENCE_ENALBE       = ENABLE;
const DPL::String BACKBUTTON_PRESENCE_DISABLE      = DISABLE;
const DPL::String CONTEXT_MENU_ENABLE              = ENABLE;
const DPL::String CONTEXT_MENU_DISABLE             = DISABLE;
const DPL::String ENCRYPTION_ENABLE                = ENABLE;
const DPL::String ENCRYPTION_DISABLE               = DISABLE;
const DPL::String PROGRESSBAR_PRESENCE_ENABLE      = ENABLE;
const DPL::String PROGRESSBAR_PRESENCE_DISABLE     = DISABLE;
const DPL::String HWKEY_EVENT_ENABLE               = ENABLE;
const DPL::String HWKEY_EVENT_DISABLE              = DISABLE;
const DPL::String ACCESSIBILITY_ENABLE             = ENABLE;
const DPL::String ACCESSIBILITY_DISABLE            = DISABLE;
const DPL::String SOUND_MODE_SAHRED                = L"shared";
const DPL::String SOUND_MODE_EXCLUSIVE             = L"exclusive";
} // namespace Value
} // namespace TizenSetting

enum WidgetSettingScreenLock
{
    Screen_Portrait, /* Default */
    Screen_Landscape,
    Screen_AutoRotation
};

enum WidgetSettingIndicatorPresence
{
    Indicator_Enable,     /* Default */
    Indicator_Disable
};

enum WidgetSettingBackButtonPresence
{
    BackButton_Enable,
    BackButton_Disable     /* Default */
};

enum WidgetSettingContextMenu
{
    ContextMenu_Enable,  /* Default */
    ContextMenu_Disable
};

enum WidgetSettingEncryption
{
    Encryption_Enable,
    Encryption_Disable     /* Default */
};

enum WidgetSettingBackgroundSupport
{
    BackgroundSupport_Enable,
    BackgroundSupport_Disable    /* Default */
};

enum WidgetSettingProgressBarPresence
{
    ProgressBar_Enable,
    ProgressBar_Disable    /* Default */
};

enum WidgetSettingHWkeyEventPresence
{
    HWkeyEvent_Enable,    /* Default */
    HWkeyEvent_Disable
};

enum WidgetSettingAccessibility
{
    Accessibility_Enable,    /* Default */
    Accessibility_Disable
};

enum WidgetSettingSoundMode
{
    SoundMode_Shared,    /* Default */
    SoundMode_Exclusive
};

enum WidgetSettingBackgroundVibration
{
    BackgroundVibration_Enable,
    BackgroundVibration_Disable    /* Default */
};

class WidgetSettingList
{
  public:
    WidgetSettingList();
    WidgetSettingList(WrtDB::WidgetSettings &widgetSettings);

    WidgetSettingScreenLock getRotationValue() const;
    WidgetSettingIndicatorPresence getIndicatorPresence() const;
    WidgetSettingBackButtonPresence getBackButtonPresence() const;
    WidgetSettingContextMenu getContextMenu() const;
    WidgetSettingEncryption getEncryption() const;
    WidgetSettingBackgroundSupport getBackgroundSupport() const;
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
    std::string getUserAgent() const;
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
    WidgetSettingProgressBarPresence getProgressBarPresence() const;
    WidgetSettingHWkeyEventPresence getHWkeyEvent() const;
    WidgetSettingAccessibility getAccessibility() const;
    WidgetSettingSoundMode getSoundMode() const;
    WidgetSettingBackgroundVibration getBackgroundVibration() const;
    bool operator ==(const WidgetSettingList& other) const;

  private:
    void displayError(const DPL::String& name, const DPL::String& value);

    bool m_logEnable;
    WidgetSettingScreenLock m_RotationLock;
    WidgetSettingIndicatorPresence m_IndicatorPresence;
    WidgetSettingBackButtonPresence m_BackButtonPresence;
    WidgetSettingContextMenu m_ContextMenu;
    WidgetSettingEncryption m_Encryption;
    WidgetSettingBackgroundSupport m_BackgroundSupport;
    WidgetSettingProgressBarPresence m_ProgressbarPresence;
    WidgetSettingHWkeyEventPresence m_HWkeyEvent;
    WidgetSettingAccessibility m_Accessibility;
    WidgetSettingSoundMode m_SoundMode;
    WidgetSettingBackgroundVibration m_BackgroundVibration;
#if ENABLE(CUSTOM_USER_AGENT_SUPPORT)
    std::string m_UserAgent;
#endif // ENABLE(CUSTOM_USER_AGENT_SUPPORT)
};
typedef std::shared_ptr<WidgetSettingList> WidgetSettingListPtr;

namespace OrientationAngle {
namespace W3C {
namespace Portrait {
const int PRIMARY  = 0;
const int SECONDARY = 180;
} // namespace Portrait
namespace Landscape {
const int PRIMARY  = 90;
const int SECONDARY = -90;
} // namespace Landscape
} // namespace W3C
namespace Window {
namespace Portrait {
const int PRIMARY  = 0;
const int SECONDARY = 180;
} // namespace Portrait
namespace Landscape {
const int PRIMARY  = 270;
const int SECONDARY = 90;
} // namespace Landscape
const int UNLOCK = -1;
} // namespace Window
} // namespace OrientationAngle

namespace KeyName {
const std::string BACK = "back";
const std::string MENU = "menu";
} // KeyName

enum ConsoleLogLevel {
    Debug = 0,
    Warning = 1,
    Error = 2,
};
#endif /* WRT_SRC_DOMAIN_WIDGET_DATA_TYPES_H_ */
