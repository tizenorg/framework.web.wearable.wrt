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
 * @file    client_ide_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */
#ifndef CLIENT_IDE_SUPPORT_H_
#define CLIENT_IDE_SUPPORT_H_

#include <bundle.h>

namespace ClientModule {
namespace IDESupport {
bool getDebugMode(bundle* b);
bool sendReply(bundle* b, unsigned int portNum);
void consoleMessage(int level, const char* format, ...);
} // IDESupport
} // ClientModule
#endif // CLIENT_IDE_SUPPORT_H_