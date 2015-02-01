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
 * @file    view_logic_storage_support.cpp
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @brief   Implementation file of StorageSupport API used by ViewLogic
 */
#include "view_logic_storage_support.h"

#include <string>
#include <ftw.h>

#include <dpl/assert.h>
#include <dpl/exception.h>
#include <dpl/log/log.h>
#include <dpl/string.h>
#include <dpl/utils/wrt_utility.h>
#include <widget_model.h>

namespace ViewModule {
namespace StorageSupport {
namespace { //anonymous
const mode_t TEMPORARY_STORAGE_MODE = 0700;
static bool rootDirectory = true;

static int removeFile(const char* path, const struct stat* /*sb*/, int tflag)
{
    if (path == NULL) {
        LogError("Wrong input path");
        return 0;
    }
    LogDebug(path);
    std::string inputPath = path;

    if (rootDirectory) {
        LogDebug("Skip root directory");
        rootDirectory = false;
        return 0;
    }

    if (tflag == FTW_F || tflag == FTW_D) {
        if (!WrtUtilRemove(inputPath)) {
            LogError("Fail to remove");
        }
    } else if (tflag == FTW_DNR) {
        LogError("This is directory which can't be read");
    } else if (tflag == FTW_NS) {
        LogError("Unknow error");
    }

    return 0;
}

bool removeDirectory(const char* path)
{
    rootDirectory = true;
    if (ftw(path, removeFile, 20) != 0) {
        return false;
    }
    return true;
}
}

void initializeStorage(WidgetModel *widgetModel)
{
    LogDebug("initializeStorage");
    AssertMsg(widgetModel, "Passed widgetModel is NULL!");

    // create temporary storage
    std::string path =
        DPL::ToUTF8String(widgetModel->TemporaryStoragePath.Get());
    if (!WrtUtilDirExists(path)) {
        if (!WrtUtilMakeDir(path, TEMPORARY_STORAGE_MODE)) {
            ThrowMsg(DPL::CommonException::InternalError,
                     "Fail to initialize temporary storage");
        }
    } else {
        if (!removeDirectory(path.c_str())) {
            LogError("Failed to clean temporary storage");
        }
    }
}

void deinitializeStorage(WidgetModel *widgetModel)
{
    LogDebug("deinitializeStorage");
    AssertMsg(widgetModel, "Passed widgetModel is NULL!");

    // clean-up temporary storage
    std::string path =
        DPL::ToUTF8String(widgetModel->TemporaryStoragePath.Get());
    if (!removeDirectory(path.c_str())) {
        LogError("Fail to deinitialize temporary storage");
    }
}
} // namespace StorageSupport
} // namespace ViewModule
