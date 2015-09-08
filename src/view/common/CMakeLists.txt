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

# @file     src/view/common/CMakeLists.txt
# @author   Krzysztof Jackiewicz (k.jackiewicz@samsung.com)

PKG_CHECK_MODULES(SYS_VIEW_COMMON_DEP
    wrt-commons-custom-handler-dao-ro
    appsvc
    aul
    cert-svc-vcore
    capi-appfw-app-manager
    efl-assist
    eina
    elementary
    evas
#   haptic
    deviced
    libcurl    # uri unescape
    libiri
    libpcrecpp
    libsoup-2.4
    notification
    security-client
    security-core
    secure-storage
    REQUIRED
)

PKG_CHECK_MODULES(VIEW_COMMON_DEP
    dpl-efl
    dpl-event-efl
    dpl-wrt-dao-ro
    wrt-commons-security-origin-dao
    wrt-commons-certificate-dao
    wrt-popup-wrt-runner
    REQUIRED
)

SET(VIEW_COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/src/view/common/application_launcher.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/evas_object.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/scheme.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/scheme_action_map.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_apps_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_custom_header_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_help_popup_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_security_origin_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_security_origin_support_util.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_security_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_storage_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_uri_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_vibration_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_certificate_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/view_logic_privilege_support.cpp
)

SET(VIEW_COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/domain
    ${PROJECT_SOURCE_DIR}/src/profiling
    ${PROJECT_SOURCE_DIR}/src/view/common
    ${VIEW_COMMON_DEP_INCLUDE_DIRS}
    )

INCLUDE_DIRECTORIES(${VIEW_COMMON_INCLUDES})
INCLUDE_DIRECTORIES(SYSTEM ${SYS_VIEW_COMMON_DEP_INCLUDE_DIRS})

ADD_LIBRARY(${TARGET_VIEW_COMMON_LIB_STATIC} STATIC
    ${VIEW_COMMON_SOURCES}
)

TARGET_LINK_LIBRARIES(${TARGET_VIEW_COMMON_LIB_STATIC}
    ${VIEW_COMMON_DEP_LIBRARIES}
    ${SYS_VIEW_COMMON_DEP_LIBRARIES}
    ${PROF_LIB}
)

SET_TARGET_PROPERTIES(${TARGET_VIEW_COMMON_LIB_STATIC} PROPERTIES
    COMPILE_DEFINITIONS LOG_TAG="${LOG_TAG}")

SET_TARGET_PROPERTIES(${TARGET_VIEW_COMMON_LIB_STATIC} PROPERTIES
    SOVERSION ${PROJECT_API_VERSION}
    VERSION ${PROJECT_VERSION}
    )
SET_TARGET_PROPERTIES(${TARGET_VIEW_COMMON_LIB_STATIC} PROPERTIES
    COMPILE_FLAGS -fPIC)
SET_TARGET_PROPERTIES(${TARGET_VIEW_COMMON_LIB_STATIC} PROPERTIES
    COMPILE_FLAGS "-include profiling_util.h"
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both"
    OUTPUT_NAME ${TARGET_VIEW_COMMON_LIB_STATIC})
