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
# @file        CMakeLists.txt
# @author      Tomasz Iwanek (t.iwanek@samsung.com)
#

SET(WRT_TESTS_GENERAL_WIDGETS_PATH "/opt/share/widget/tests/general/")

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/any.wgt DESTINATION ${WRT_TESTS_GENERAL_WIDGETS_PATH})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/minimal.wgt DESTINATION ${WRT_TESTS_GENERAL_WIDGETS_PATH})
