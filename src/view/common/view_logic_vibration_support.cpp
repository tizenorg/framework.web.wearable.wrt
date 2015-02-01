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
 * @file    view_logic_vibration_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "view_logic_vibration_support.h"
#include <dpl/log/log.h>
#include <dd-haptic.h>

namespace ViewModule {
VibrationSupport::VibrationSupport() : m_initialized(false),
    m_handle(NULL),
    m_effect_handle(NULL)
{}

VibrationSupport::~VibrationSupport()
{}

void VibrationSupport::initialize(void)
{
    LogDebug("initialize");
    initializeVibration();
}

void VibrationSupport::deinitialize(void)
{
    LogDebug("deinitialize");

    if (m_initialized) {
        int ret = haptic_close(m_handle);

        if (HAPTIC_ERROR_NONE == ret) {
            m_initialized = false;
            LogDebug("deinitialize success");
        } else {
            m_initialized = true;
            LogDebug("deinitialize failed - error code : " << ret);
        }
    }
}

void VibrationSupport::startVibration(const long vibrationTime)
{
    LogDebug("startVibration called");

    if (m_initialized == false) {
        if (initializeVibration() == false) {
            return;
        }
    }

    int time = static_cast<int>(vibrationTime);
    int ret = haptic_vibrate_monotone(m_handle, time, &m_effect_handle);

    if (HAPTIC_ERROR_NONE == ret) {
        LogDebug("haptic_vibrate_monotone success");
    } else {
        LogDebug("haptic_vibrate_monotone failed - error code : " << ret);
    }

    return;
}

void VibrationSupport::stopVibration(void)
{
    LogDebug("stopVibration called");

    if (m_initialized == false) {
        if (initializeVibration() == false) {
            return;
        }
    }

    int ret = haptic_stop_all_effects(m_handle);

    if (HAPTIC_ERROR_NONE == ret) {
        LogDebug("haptic_stop_all_effects success");
    } else {
        LogDebug("haptic_stop_all_effects failed - error code : " << ret);
    }

    return;
}

bool VibrationSupport::initializeVibration(void)
{
    LogDebug("initializeVibration called");

    if (m_initialized == false) {
        haptic_device_h handle = NULL;
        int ret = haptic_open(HAPTIC_DEVICE_0, &handle);

        if (ret == HAPTIC_ERROR_NONE) {
            LogDebug("initializeVibration success");
            m_initialized = true;
            m_handle = handle;
        } else {
            LogDebug("initializeVibration failed - error code : " << ret);
            m_initialized = false;
            m_handle = NULL;
        }
    }

    return m_initialized;
}
} // namespace ViewModule
