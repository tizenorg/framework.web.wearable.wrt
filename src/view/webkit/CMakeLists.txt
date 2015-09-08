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

# @file     src/view/webkit/CMakeLists.txt
# @author   Pawel Sikorski (p.sikorski@samsung.com)

ADD_DEFINITIONS("-DWRT_LOG")

include(FindPkgConfig)

PKG_CHECK_MODULES(VIEW_MODULE_DEP
    dpl-dbus-efl
    dpl-utils-efl
    dpl-wrt-dao-ro
    notification
    wrt-commons-custom-handler-dao-rw
    wrt-dispatch-event
    wrt-plugins-ipc-message
    wrt-plugin-js-overlay
    wrt-popup-wrt-runner
    security-core
    security-client
#    haptic
    deviced
    capi-system-system-settings
    efl-assist
    privacy-manager-client
    REQUIRED
)

PKG_CHECK_MODULES(SYS_VIEW_MODULE_DEP
    appsvc
    capi-appfw-app-manager
    eina
    ewebkit2
    REQUIRED
)

SET(VIEW_MODULE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/view/webkit/ewk_context_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_authentication_request_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_certificate_confirm_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_geolocation_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_message_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_orientation_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_scheme_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_usermedia_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_web_notification_data.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_web_notification_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_web_notification_permission_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/view_logic_web_storage_support.cpp
    ${VIEW_MODULE_SOURCES}
)

SET(VIEW_MODULE_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/api_new
    ${PROJECT_SOURCE_DIR}/src/domain
    ${PROJECT_SOURCE_DIR}/src/profiling
    ${PROJECT_SOURCE_DIR}/src/view
    ${PROJECT_SOURCE_DIR}/src/view/webkit
    ${PROJECT_SOURCE_DIR}/src/plugin-service
    ${VIEW_MODULE_DEP_INCLUDE_DIRS}
)

ADD_DEFINITIONS(${VIEW_MODULE_DEP_CFLAGS})

INCLUDE_DIRECTORIES(${VIEW_MODULE_INCLUDES})
INCLUDE_DIRECTORIES(SYSTEM ${SYS_VIEW_MODULE_DEP_INCLUDE_DIRS})

ADD_LIBRARY(${TARGET_VIEW_MODULE_LIB} STATIC
    ${VIEW_MODULE_SOURCES}
)

TARGET_LINK_LIBRARIES(${TARGET_VIEW_MODULE_LIB}
    ${VIEW_MODULE_DEP_LIBRARIES}
    ${PROF_LIB}
#    ${TARGET_PLUGIN_MODULE_LIB}
)

SET_TARGET_PROPERTIES(${TARGET_VIEW_MODULE_LIB} PROPERTIES
    COMPILE_DEFINITIONS LOG_TAG="${LOG_TAG}")

SET_TARGET_PROPERTIES(${TARGET_VIEW_MODULE_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_API_VERSION})
SET_TARGET_PROPERTIES(${TARGET_VIEW_MODULE_LIB} PROPERTIES
    COMPILE_FLAGS "-include profiling_util.h"
    OUTPUT_NAME ${TARGET_VIEW_MODULE_LIB}
)

ADD_SUBDIRECTORY(injected-bundle)
