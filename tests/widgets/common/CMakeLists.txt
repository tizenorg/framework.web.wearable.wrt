# Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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
# @version     1.0
# @brief
#

SET(COMMON_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")

WRT_TEST_LIBRARY(${WRT_TEST_LIBRARY})

WRT_INCLUDE_DIRECTORIES(
    ${COMMON_LIB_PKGS_INCLUDE_DIRS}
    ${COMMON_INCLUDES}
    ${WRT_API_NEW_INCLUDES}
    )

WRT_LINK_DIRECTORIES(${COMMON_LIB_PKGS_LIBRARY_DIRS})
WRT_TARGET_LINK_LIBRARIES(${COMMON_LIB_PKGS_LIBRARIES})

SET(WRT_DETAIL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InstallerWrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RunnableObjectStateTester.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mock/MockViewModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mock/MockContextManager.cpp
)

INCLUDE_DIRECTORIES(${COMMON_INCLUDES})
INCLUDE_DIRECTORIES(${COMMON_LIB_PKGS_INCLUDE_DIRS})
ADD_LIBRARY(${WRT_TEST_LIBRARY} STATIC ${WRT_DETAIL_SOURCES})
