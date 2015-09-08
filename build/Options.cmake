# Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#
# @file        Options.cmake
# @author      Jihoon Chung (jihoon.chung@samsung.com)
#

# Macro declaration
#  - WRT_OPTION() : Wrapper omitting description argument from OPTION() command.
MACRO(WRT_OPTION feature_name on_off)
    OPTION(${feature_name} "" ${on_off})
ENDMACRO(WRT_OPTION)

# Use Feature
#   Use a particular optional platform service or third-party library
#
# Description : <text>
#               <text>
# Author : <text>(<email>) - <date>
# WRT_OPTION(<REPOSITORY_NAME>_USE_<FEATURE_NAME> ON/OFF)
#WRT_OPTION(WRT_USE_WEBKIT_UPVERSION OFF)

# Enable Feature
#   Turn on a specific feature of WRT
#
# Description : <text>
#               <text>
# Author : <text>(<email>) - <date>
# WRT_OPTION(<REPOSITORY_NAME>_ENABLE_<FEATURE_NAME> ON/OFF)

