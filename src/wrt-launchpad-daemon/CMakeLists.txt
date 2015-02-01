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
# @author   Tae-Jeong Lee (taejeong.lee@samsung.com)
# @version     0.1
#

SET(WRT_LAUNCH_PAD_NAME "wrt_launchpad_daemon")

ADD_DEFINITIONS("-DSHARE_PREFIX=\"/usr/share/aul\"")
ADD_DEFINITIONS("-DUSE_POOL_FOR_FIRST_LAUNCH")

PKG_CHECK_MODULES(WRT_LAUNCH_PAD_DEPS
    aul
    dlog
    app-checker
    bundle
    dbus-glib-1
    glib-2.0
    libsmack
    libprivilege-control
    x11
    sqlite3
    libsystemd-daemon
    REQUIRED
)

SET(WRT_LAUNCH_PAD_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src/wrt-launchpad-daemon/include
    ${PROJECT_SOURCE_DIR}/src/wrt-launchpad-daemon/legacy
    ${PROJECT_SOURCE_DIR}/src/wrt-launchpad-daemon/feature
    ${WRT_LAUNCH_PAD_DEPS_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(${WRT_LAUNCH_PAD_INCLUDE_DIRS})

#build executable
ADD_EXECUTABLE( ${WRT_LAUNCH_PAD_NAME}
    src/app_sock.c
    src/simple_util.c
    launchpad_src/launchpad.c
    launchpad_src/util_x.c
    src/process_pool.c
)

#link libraries
TARGET_LINK_LIBRARIES( ${WRT_LAUNCH_PAD_NAME}
    ${WRT_LAUNCH_PAD_DEPS_LIBRARIES}
    ${CMAKE_DL_LIBS}
    "-lcap"
)

#target properties
SET_TARGET_PROPERTIES(${WRT_LAUNCH_PAD_NAME} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both -Wl,--version-script=${PROJECT_SOURCE_DIR}/src/wrt-launchpad-daemon/${WRT_LAUNCH_PAD_NAME}.map"
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH_USE_LINK_PATH ON
)

#install
INSTALL(TARGETS ${WRT_LAUNCH_PAD_NAME} DESTINATION bin)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/legacy/preload_list_wrt.txt DESTINATION /usr/share/aul )
