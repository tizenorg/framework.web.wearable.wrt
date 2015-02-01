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
 * @file    client_command_line_parser.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#include "client_command_line_parser.h"

#include <cstddef>
#include <sstream>
#include <string>

#include <dpl/log/log.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

namespace ClientModule {
namespace {

std::string parseIdField(int argc, char **argv)
{
    if (argv[0] == NULL) {
        return "";
    }

    std::string arg = argv[0];
    if (arg.empty()) {
        return "";
    }

    if (arg.find("wrt-client") != std::string::npos) {
        if (argc <= 1) {
            return "";
        }

        arg = argv[1];

        if (arg == "-h" || arg == "--help") {
            return "";
        } else if (arg == "-l" ||
                   arg == "--launch" ||
                   arg == "-t" ||
                   arg == "--tizen")
        {
            if (argc != 3) {
                return "";
            }
            return argv[2];
        } else {
            return "";
        }
    } else {
        std::size_t pos = arg.find_last_of('/');
        if (pos != std::string::npos) {
            arg = arg.erase(0, pos + 1);
        }
        return arg;
    }
}

DPL::OptionalUInt getIndex(const std::string& tizenId)
{
    std::size_t pos =
        tizenId.find(WrtDB::AppControlPrefix::PROCESS_PREFIX);
    if (pos != std::string::npos) {
        std::string index = tizenId.substr(pos);
        index.erase(strlen(WrtDB::AppControlPrefix::PROCESS_PREFIX));
        std::stringstream s(index);
        unsigned int appControlIndex;
        s >> appControlIndex;
        return appControlIndex;
    }
    return DPL::OptionalUInt();
}

std::string removeIndex(const std::string& tizenId)
{
    std::string id = tizenId;
    std::size_t pos =
        id.find(WrtDB::AppControlPrefix::PROCESS_PREFIX);
    if (pos != std::string::npos) {
        id.erase(pos);
    }
    return id;
}
}

std::string CommandLineParser::getTizenId(int argc, char **argv)
{
    std::string id = parseIdField(argc, argv);
    if (id.empty()) {
        return "";
    }
    return removeIndex(id);
}

DPL::OptionalUInt CommandLineParser::getAppControlIndex(int argc, char **argv)
{
    std::string id = parseIdField(argc, argv);
    if (id.empty()) {
        return DPL::OptionalUInt();
    }
    return getIndex(id);
}
} // ClientModule
