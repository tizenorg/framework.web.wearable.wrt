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

# @file     src/view/webkit/injected-bundle/CMakeLists.txt
# @author   Lukasz Wrzosek (l.wrzosek@samsung.com)

ADD_DEFINITIONS("-DWRT_BUNDLE_LOG")

include(FindPkgConfig)

SET(INJECTED_BUNDLE_DEP_PACKAGES
    dpl-efl
    dpl-utils-efl
    dpl-wrt-dao-ro
    ewebkit2
    libprivilege-control
    libiri
    libpcrecpp
    wrt-plugins-ipc-message
    wrt-dispatch-event
    pkgmgr-info
    vconf
    libresourced
)

PKG_CHECK_MODULES(INJECTED_BUNDLE_DEP
    ${INJECTED_BUNDLE_DEP_PACKAGES}
    REQUIRED
)

SET(INJECTED_BUNDLE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/view/webkit/injected-bundle/wrt-injected-bundle.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/injected-bundle/injected_bundle_uri_handling.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/injected-bundle/injected_bundle_decryption_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/injected-bundle/injected_bundle_viewmodes_support.cpp
    ${PROJECT_SOURCE_DIR}/src/view/webkit/injected-bundle/page_global_context_container.cpp
    ${PROJECT_SOURCE_DIR}/src/view/common/scheme.cpp
)

SET(INJECTED_BUNDLE_INCLUDES
    ${INJECTED_BUNDLE_DEP_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/plugin-service
    ${PROJECT_SOURCE_DIR}/src/view/common
    ${PROJECT_SOURCE_DIR}/src/domain
)

ADD_DEFINITIONS(${INJECTED_BUNDLE_DEP_CFLAGS})
INCLUDE_DIRECTORIES(
    ${INJECTED_BUNDLE_INCLUDES}
    ${SMACK_LABELING_SUPPORT_INCLUDES}
    )

ADD_LIBRARY(${TARGET_INJECTED_BUNDLE_LIB} SHARED
    ${INJECTED_BUNDLE_SOURCES}
)

TARGET_LINK_LIBRARIES(
    ${TARGET_INJECTED_BUNDLE_LIB}
    ${INJECTED_BUNDLE_DEP_LIBRARIES}
    ${TARGET_WRT_ENGINE_STATIC}
    ${TARGET_PLUGIN_MODULE_LIB}
    ${PROF_LIB}
    ${SMACK_LABELING_SUPPORT_STATIC}
)

# for encryption
TARGET_LINK_LIBRARIES(${TARGET_INJECTED_BUNDLE_LIB} "-lss-client")

# Overriden log tag!
SET_TARGET_PROPERTIES(${TARGET_INJECTED_BUNDLE_LIB} PROPERTIES
    COMPILE_DEFINITIONS LOG_TAG="WRT_BUNDLE")

SET_TARGET_PROPERTIES(${TARGET_INJECTED_BUNDLE_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_API_VERSION})
SET_TARGET_PROPERTIES(${TARGET_INJECTED_BUNDLE_LIB} PROPERTIES
    COMPILE_FLAGS -fPIC)
SET_TARGET_PROPERTIES(${TARGET_INJECTED_BUNDLE_LIB} PROPERTIES
    LINK_FLAGS "-Wl,--as-needed -Wl,--hash-style=both -Wl,--version-script=${PROJECT_SOURCE_DIR}/src/view/webkit/injected-bundle/${TARGET_INJECTED_BUNDLE_LIB}.map"
    OUTPUT_NAME ${TARGET_INJECTED_BUNDLE_LIB}
)

INSTALL(TARGETS ${TARGET_INJECTED_BUNDLE_LIB}
    DESTINATION lib/
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

