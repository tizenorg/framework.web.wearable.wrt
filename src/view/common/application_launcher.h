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
 * @file    application_launcher.h
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @version 1.0
 * @brief   Header file for application launcher
 */

#ifndef APPLICATION_LAUNCHER_H
#define APPLICATION_LAUNCHER_H

#include <string>
#include <dpl/event/controller.h>
#include <dpl/generic_event.h>
#include <dpl/singleton.h>

#include <appfw/app_control.h>

namespace ApplicationLauncherEvents {
DECLARE_GENERIC_EVENT_3(LaunchApplicationByAppService,
                        app_control_h,
                        app_control_reply_cb,
                        void*)
DECLARE_GENERIC_EVENT_4(LaunchApplicationByPkgname,
                        std::string,
                        std::string,
                        std::string,
                        std::string)
} //namespace ApplicationLauncherEvents

namespace ApplicationLauncherPkgname {
const std::string PKG_NAME_PREFIX = "com.samsung.";
const std::string PKG_NAME_DOWNLOAD_PROVIDER = PKG_NAME_PREFIX +
    "download-provider";
const std::string PKG_NAME_VIDEO_PLAYER = PKG_NAME_PREFIX + "video-player";
const std::string PKG_NAME_VT_MAIN = PKG_NAME_PREFIX + "vtmain";
} // namespace ApplicationLauncherPkgname

class ApplicationLauncher :
    public DPL::Event::Controller<DPL::TypeListDecl<
                                      ApplicationLauncherEvents::
                                          LaunchApplicationByPkgname,
                                      ApplicationLauncherEvents::
                                          LaunchApplicationByAppService
                                      >::Type>
{
  public:
    ApplicationLauncher();
    virtual ~ApplicationLauncher();
    void setWidgetTizenId(const std::string& tizenId);
    void setWindowHandle(unsigned windowHandle);

  protected:
    virtual void OnEventReceived(
        const ApplicationLauncherEvents::
            LaunchApplicationByPkgname &event);
    virtual void OnEventReceived(
        const ApplicationLauncherEvents::
            LaunchApplicationByAppService &event);

  private:
    std::string m_tizenId;
    unsigned m_windowHandle;
};

typedef DPL::Singleton<ApplicationLauncher> ApplicationLauncherSingleton;

#endif //APPLICATION_LAUNCHER_H
