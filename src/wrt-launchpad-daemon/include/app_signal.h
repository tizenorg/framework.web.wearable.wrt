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

#ifndef __APP_DBUS_H__
#define __APP_DBUS_H__

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#define AUL_DBUS_PATH "/aul/dbus_handler"
#define AUL_DBUS_SIGNAL_INTERFACE "com.samsung.aul.signal"
#define AUL_DBUS_APPDEAD_SIGNAL "app_dead"
#define AUL_DBUS_APPLAUNCH_SIGNAL       "app_launch"


#define SYSTEM_BUS_NAME       "org.tizen.system.deviced"
#define SYSTEM_OBJECT_PATH    "/Org/Tizen/System/DeviceD/PmQos"
#define SYSTEM_INTERFACE_NAME "org.tizen.system.deviced.PmQos"
#define SYSTEM_METHOD_NAME    "WebappLaunch"

#define DBUS_WRT_OBJECT_PATH "/Org/Tizen/Runtime/WRT"
#define DBUS_WRT_INTERFACE_NAME "org.tizen.runtime.wrt.status"
#define DBUS_WRT_SIGNAL_READYDONE "READYDONE"

#endif
