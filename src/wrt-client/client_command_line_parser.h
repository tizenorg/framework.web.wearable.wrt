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
 * @file    client_command_line_parser.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */
#ifndef CLIENT_COMMAND_LINE_PARSER_H_
#define CLIENT_COMMAND_LINE_PARSER_H_

#include <dpl/optional_typedefs.h>

namespace ClientModule {
namespace CommandLineParser {
std::string getTizenId(int argc, char **argv);
DPL::OptionalUInt getAppControlIndex(int argc, char **argv);
} // CommandLineParser
} // ClientModule
#endif // CLIENT_COMMAND_LINE_PARSER_H_