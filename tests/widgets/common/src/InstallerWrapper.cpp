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

#include "InstallerWrapper.h"

#include <dpl/log/log.h>

#include <cstdio>

namespace
{

const std::string params = "DPL_USE_OLD_STYLE_LOGS=0 "
    "DPL_USE_OLD_STYLE_PEDANTIC_LOGS=0 WRT_TEST_MODE=1 ";
const std::string installCmd = params + "wrt-installer -i ";
const std::string uninstallCmd = params + "wrt-installer -un ";
const std::string uninstallByGuidCmd = params + "wrt-installer -ug \"";
const std::string redirection =  " 2>&1";
const std::string INSTALLER_MESSAGE_ID_LINE =
    "## wrt-installer : %s installation was successful.\n";
const std::string INSTALLER_MESSSGE_START = "## wrt-installer : ";

std::string getAndCutInstallerLogLine(std::string &src)
{
    size_t startIndex = src.find(INSTALLER_MESSSGE_START);
    if (startIndex == std::string::npos)
    {
        LogWarning("Installer message can not be found");
        return std::string();
    }
    size_t newLineIndex = src.find("\n", startIndex);
    std::string line = src.substr(startIndex, newLineIndex - startIndex + 1);
    src.erase(0, newLineIndex + 1);
    return line;
}

}

namespace InstallerWrapper
{

InstallResult install(
        const std::string& path,
        std::string& tizenId,
        const std::string& user)
{
    std::string msg;

    auto cmd = installCmd + path + redirection;
    if(user.length()) //if other user should be used
    {
        cmd = "su " + user + " -c '" + cmd + "'";
    }
    LogDebug("executing: " << cmd);
    auto filehandle = popen(cmd.c_str(), "r");
    if (!filehandle) {
        return OtherError;
    }

    char buffer[1024] = "";
    while (fread_unlocked(buffer, sizeof(char), sizeof(buffer)/sizeof(char),
                                 filehandle) > 0)
    {
        msg += buffer;
    }
    LogDebug(msg);
    auto err = pclose(filehandle);
    if (!WIFEXITED(err)) {
        return OtherError;
    }
    if (0 != WEXITSTATUS(err)) {
        if (1 == WEXITSTATUS(err)) {
            return WrongWidgetPackage;
        }
        return OtherError;
    }

    char* id = NULL;
    std::string line;

    while ((line = getAndCutInstallerLogLine(msg)) != "")
    {
        if (line.find("successful") != std::string::npos)
        {
            id = new char[line.length()];
            int nr = sscanf(line.c_str(), INSTALLER_MESSAGE_ID_LINE.c_str(), id);

            if (1 != nr)
            {
                LogWarning("Can not read widget ID from message: " << line);
                delete[] id;
                return OtherError;
            }
            tizenId = id;
            delete[] id;
            if (tizenId != "plugin")
            {
                return Success;
            }
        }
    }

    return OtherError;
}

bool uninstall(const std::string& tizenId)
{
    std::string cmd = uninstallCmd + tizenId + " > /dev/null 2>/dev/null";
    LogDebug("executing: " << cmd);
    return (system(cmd.c_str()) == EXIT_SUCCESS);
}

bool uninstallByGuid(const std::string& guid)
{
    std::string cmd = uninstallByGuidCmd + guid + "\" > /dev/null 2>/dev/null";
    LogDebug("executing: " << cmd);
    return (system(cmd.c_str()) == EXIT_SUCCESS);
}

bool sigintWrtClients()
{
    return (system("pkill -2 wrt-client") == 0);
}

}

