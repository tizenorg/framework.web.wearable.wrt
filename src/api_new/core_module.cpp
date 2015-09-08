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
 * @file    core_module.cpp
 * @author  Przemyslaw Ciezkowski (p.ciezkowski@samsung.com)
 * @author  Andrzej Surdej (a.surdej@samsung.com)
 * @version 1.0
 * @brief   File contains definitions of wrt core module.
 */

#include "core_module.h"
#include "runnable_widget_object.h"
#include <string>
#include <main_thread.h>
#include <dpl/log/log.h>
#include <dpl/assert.h>
#include <dpl/exception.h>
#include <dpl/singleton_impl.h>
#include <dpl/optional_typedefs.h>
#include "localization_setting.h"
#include <dpl/wrt-dao-ro/global_config.h>
#include <profiling_util.h>
#include <widget_deserialize_model.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

IMPLEMENT_SINGLETON(WRT::CoreModule)

namespace {
const char* const TEXT_DOMAIN = "wrt";
const char* const TEXT_LOCALE_PATH = "/usr/share/locale";

std::string cutOffFileName(const std::string& path)
{
    size_t found = path.find_last_of("/");
    if (found == std::string::npos) {
        return path;
    } else {
        return path.substr(0, found);
    }
}

bool isDir(const std::string& path)
{
    struct stat st;
    if (0 == stat(path.c_str(), &st) && S_ISDIR(st.st_mode)) {
        return true;
    }
    LogError("Cannot access directory [ " << path << " ]");
    return false;
}

bool checkPaths()
{
    using namespace WrtDB;
    using namespace WrtDB::GlobalConfig;

    bool if_ok = true;
    if_ok &= (isDir(cutOffFileName(GetWrtDatabaseFilePath())));
    if_ok &= (isDir(GetDevicePluginPath()));
    if_ok &= (isDir(GetUserInstalledWidgetPath()));
    return if_ok;
}
} // namespace anonymous

namespace WRT {
class CoreModuleImpl
{
  public:

    CoreModuleImpl() : m_initialized(false)
    {
        LogDebug("enter");
    }

    ~CoreModuleImpl()
    {
        LogDebug("Core module implementation destroyed");
    }

    bool Init()
    {
        if (!m_initialized) {
            ADD_PROFILING_POINT("CoreModule::Init", "point");
            DPL::Log::LogSystemSingleton::Instance().SetTag("WRT");
            LogDebug("Initialize");
            if (!checkPaths()) {
                LogError("Required path does not exist");
                return false;
            }
            Try
            {
                ADD_PROFILING_POINT("attach databases", "start");
                MainThreadSingleton::Instance().AttachDatabases();
                ADD_PROFILING_POINT("attach databases", "stop");
                LogDebug("Initialize finished");
            } catch (const DPL::Exception& ex) {
                LogError("Internal Error during screen preparation:");
                DPL::Exception::DisplayKnownException(ex);
                /* TODO:
                 * Do deinitialization: check on which step exception occured
                 * and deinitialize only initialized parts.
                 */
                return false;
            }
            bindtextdomain(TEXT_DOMAIN, TEXT_LOCALE_PATH);
            m_initialized = true;
        }
        return true;
    }

    void Terminate()
    {
        MainThreadSingleton::Instance().DetachDatabases();
        m_initialized = false;
    }

    RunnableWidgetObjectPtr getRunnableWidgetObject(
        const std::string& tizenId)
    {
        try {
            RunnableWidgetObjectPtr runnable;
            WidgetModelPtr model =
                Domain::deserializeWidgetModel(tizenId);
            if (!!model) {
                runnable.reset(new RunnableWidgetObject(model));
            }
            return runnable;
        } catch (WrtDB::WidgetDAOReadOnly::Exception::WidgetNotExist) {
            LogError("Widget not found.");
            return NULL;
        } catch (DPL::Exception) {
            LogError("Error creating runnable object");
            return NULL;
        }
    }

  private:
    bool m_initialized;
};

CoreModule::CoreModule() : m_impl(new CoreModuleImpl())
{}

CoreModule::~CoreModule()
{}

bool CoreModule::Init()
{
    return m_impl->Init();
}

void CoreModule::Terminate()
{
    return m_impl->Terminate();
}

RunnableWidgetObjectPtr CoreModule::getRunnableWidgetObject(
    const std::string& tizenId)
{
    return m_impl->getRunnableWidgetObject(tizenId);
}

} /* namespace WRT */
