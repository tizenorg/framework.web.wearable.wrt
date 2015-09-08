# Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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

# @file     CMakeLists.txt
# @author   Lukasz Marek (l.marek@samsung.com)
# @author   Yunchan Cho (yunchan.cho@samgsung.com)
# @version  1.0
#

ADD_DEFINITIONS("-DWRT_PLUGINS_COMMON_LOG")

PKG_CHECK_MODULES(WRT_PLUGIN_DEP
    wrt-plugin-loading
    wrt-plugin-js-overlay
    REQUIRED)

set(PLUGIN_MODULE_SRC_DIR ${PROJECT_SOURCE_DIR}/src/plugin-service)

set(PLUGIN_MODULE_SOURCES
    ${PLUGIN_MODULE_SRC_DIR}/wrt_plugin_module.cpp
)
ADD_DEFINITIONS(${WRT_PLUGIN_DEP_CFLAGS})
INCLUDE_DIRECTORIES(
    ${PLUGIN_MODULE_SRC_DIR}
    ${WRT_PLUGIN_DEP_INCLUDE_DIRS}
)

ADD_LIBRARY(${TARGET_PLUGIN_MODULE_LIB} SHARED
    ${PLUGIN_MODULE_SOURCES}
)

SET_TARGET_PROPERTIES(${TARGET_PLUGIN_MODULE_LIB} PROPERTIES
    COMPILE_FLAGS -fPIC
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both"
)

SET_TARGET_PROPERTIES(${TARGET_PLUGIN_MODULE_LIB} PROPERTIES
    SOVERSION ${PROJECT_API_VERSION}
    VERSION ${PROJECT_VERSION}
)

SET_TARGET_PROPERTIES(${TARGET_PLUGIN_MODULE_LIB} PROPERTIES
    COMPILE_DEFINITIONS LOG_TAG="${LOG_TAG}")

target_link_libraries(${TARGET_PLUGIN_MODULE_LIB}
    ${WRT_PLUGIN_DEP_LIBRARIES}
)

INSTALL(TARGETS ${TARGET_PLUGIN_MODULE_LIB}
    DESTINATION lib)
