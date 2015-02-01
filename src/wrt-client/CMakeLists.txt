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

SET(TARGET_WRT_CLIENT "wrt-client")

SET(WRT_CLIENT_SRCS
    ${PROJECT_SOURCE_DIR}/src/wrt-client/window_data.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/client_command_line_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/client_ide_support.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/client_orientation_support.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/client_security_support.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/client_submode_support.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/splash_screen_support.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-client/wrt-client.cpp
    ${PROJECT_SOURCE_DIR}/src/wrt-launchpad-daemon/src/process_pool.c
)

PKG_CHECK_MODULES(CLIENT_DEP
    ail
    appcore-efl
    capi-appfw-application
    dpl-wrt-dao-ro
    wrt-popup-wrt-runner
    appsvc
    efl-assist
    libsystemd-daemon
    pkgmgr-info
    minicontrol-provider
    capi-system-device
    REQUIRED
)

IF(JOURNAL_LOG_SUPPORT)
    PKG_CHECK_MODULES(JOURNAL_LOG_DEP journal REQUIRED)
ENDIF(JOURNAL_LOG_SUPPORT)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/src/wrt-client
    ${PROJECT_SOURCE_DIR}/src/api_new
    ${PROJECT_SOURCE_DIR}/src/domain
    ${PROJECT_SOURCE_DIR}/src/wrt-launchpad-daemon/include
    ${CLIENT_DEP_INCLUDE_DIRS}
    ${SMACK_LABELING_SUPPORT_INCLUDES}
    ${JOURNAL_LOG_DEP_INCLUDE_DIRS}
)

ADD_DEFINITIONS("-fPIE")

ADD_EXECUTABLE(${TARGET_WRT_CLIENT}
    ${WRT_CLIENT_SRCS}
)

TARGET_LINK_LIBRARIES(${TARGET_WRT_CLIENT}
    ${CLIENT_DEP_LIBRARIES}
    ${TARGET_CORE_MODULE_LIB}
    ${SMACK_LABELING_SUPPORT_STATIC}
    ${PROF_LIB}
    "-lcap"
    "-pie"
    ${JOURNAL_LOG_DEP_LIBRARIES}
)

SET_TARGET_PROPERTIES(${TARGET_WRT_CLIENT} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both -Wl,--version-script=${PROJECT_SOURCE_DIR}/wrt-engine.map"
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH_USE_LINK_PATH ON
)

SET_TARGET_PROPERTIES(${TARGET_WRT_CLIENT} PROPERTIES
    COMPILE_FLAGS "-include profiling_util.h" )

INSTALL(TARGETS ${TARGET_WRT_CLIENT} DESTINATION bin)

