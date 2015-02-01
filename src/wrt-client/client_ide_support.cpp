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
 * @file    client_ide_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "client_ide_support.h"
#include <stdarg.h>
#include <string>
#include <dpl/log/log.h>
#include <dpl/log/secure_log.h>
#include <dpl/exception.h>

#include <dlog.h>
#include <appsvc.h>
#include <bundle.h>
#include <widget_data_types.h>

namespace ClientModule {
namespace {
const char* const KEY_DEBUG = "debug";
const char* const KEY_PORT = "port";
const char* const VALUE_TRUE = "true";

const char* const CONSOLE_MESSAGE_LOG_TAG = "ConsoleMessage";
}

bool IDESupport::getDebugMode(bundle* b)
{
    if (!b) {
        LogWarning("bundle is empty");
        return false;
    }

    const char* value = bundle_get_val(b, KEY_DEBUG);
    if (value != NULL && !strcmp(value, VALUE_TRUE)) {
        return true;
    } else {
        return false;
    }
}

bool IDESupport::sendReply(bundle* b, unsigned int portNum)
{
    bundle* request = NULL;
    if (appsvc_create_result_bundle(b, &request) != APPSVC_RET_OK) {
        LogWarning("Fail to create result bundle");
        return false;
    }

    char port[10] = {0,};
    sprintf(port, "%u", portNum);
    if (appsvc_add_data(request, KEY_PORT, port) != APPSVC_RET_OK) {
        LogWarning("Fail to add data");
        bundle_free(request);
        return false;
    }

    if (appsvc_send_result(request, APPSVC_RES_OK) != APPSVC_RET_OK) {
        LogWarning("Fail to send result");
        bundle_free(request);
        return false;
    }

    bundle_free(request);
    return true;
}

void IDESupport::consoleMessage(int level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    switch (level) {
    case ConsoleLogLevel::Debug:
        ALOG_VA(LOG_DEBUG, CONSOLE_MESSAGE_LOG_TAG, format, args);
        break;
    case ConsoleLogLevel::Warning:
        ALOG_VA(LOG_WARN, CONSOLE_MESSAGE_LOG_TAG, format, args);
        break;
    case ConsoleLogLevel::Error:
        ALOG_VA(LOG_ERROR, CONSOLE_MESSAGE_LOG_TAG, format, args);
        break;
    default:
        ALOG_VA(LOG_DEBUG, CONSOLE_MESSAGE_LOG_TAG, format, args);
        break;
    }
    va_end(args);
}
} // ClientModule
