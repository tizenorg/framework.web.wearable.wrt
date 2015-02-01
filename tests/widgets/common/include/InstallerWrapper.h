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

#ifndef WRT_TESTS_W3C_TESTS_INSTALLER_WRAPPER_H
#define WRT_TESTS_W3C_TESTS_INSTALLER_WRAPPER_H

#include <string>

namespace InstallerWrapper
{

typedef int InstallResult;
const InstallResult WrongWidgetPackage = -2;
const InstallResult OtherError = -1;
const InstallResult Success = 0;

InstallResult install(
        const std::string& path,
        std::string& tizenId,
        const std::string& user = "");
bool uninstall(const std::string& tizenId);
bool uninstallByGuid(const std::string& guid);
/**
 * @brief killWrtClients kills processes that matches 'wrt-client'
 * @return True if any client was killed
 */
bool sigintWrtClients();

}

#endif//WRT_TESTS_W3C_TESTS_INSTALLER_WRAPPER_H
