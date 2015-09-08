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

SET(LAUNCHER "wrt-launcher")

SET(LAUNCHER_SRCS
    wrt-launcher.cpp
)

PKG_CHECK_MODULES(LAUNCHER_PKGS
    dpl-efl
    dpl-db-efl
    dpl-wrt-dao-rw
    dpl-wrt-dao-ro
    capi-appfw-app-manager
    capi-appfw-application
    appsvc
    REQUIRED
)

INCLUDE_DIRECTORIES(${LAUNCHER_PKGS_INCLUDE_DIRS})

ADD_DEFINITIONS("-fPIE")

ADD_EXECUTABLE(${LAUNCHER}
    ${LAUNCHER_SRCS}
)

TARGET_LINK_LIBRARIES(${LAUNCHER}
    ${LAUNCHER_PKGS_LIBRARIES}
)

SET_TARGET_PROPERTIES(${LAUNCHER} PROPERTIES
    COMPILE_DEFINITIONS LOG_TAG="${LOG_TAG}")

SET_TARGET_PROPERTIES(${LAUNCHER} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -pie -Wl,--hash-style=both -Wl,--version-script=${PROJECT_SOURCE_DIR}/wrt-engine.map"
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH_USE_LINK_PATH ON
)

INSTALL(TARGETS ${LAUNCHER}
    DESTINATION bin
    PERMISSIONS OWNER_READ
                OWNER_WRITE
                OWNER_EXECUTE
)
