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
 * @file    view_logic_vibration_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#ifndef VIEW_LOGIC_VIBRATION_SUPPORT_H_
#define VIEW_LOGIC_VIBRATION_SUPPORT_H_

#include <dd-haptic.h>

namespace ViewModule {
class VibrationSupport
{
  public:
    VibrationSupport();
    virtual ~VibrationSupport();

    void initialize();
    void deinitialize();
    void startVibration(const long vibrationTime);
    void stopVibration(void);

  private:
    bool m_initialized;
    haptic_device_h m_handle;
    haptic_effect_h m_effect_handle;

    bool initializeVibration(void);
};
} // namespace ViewModule

#endif // VIEW_LOGIC_VIBRATION_SUPPORT_H_
