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
# @author   Lukasz Wrzosek (l.wrzosek@samsung.com)
# @version     1.0
#

ADD_DEFINITIONS("-DWRT_LOG")

# "smack-labeling-support-static" static library -------------------------------
SET(SMACK_LABELING_SUPPORT_STATIC  "smack-labeling-support-static")
PKG_CHECK_MODULES(SMACK_LABELING_SUPPORT_DEPS
    dpl-efl
    libsmack
    REQUIRED
)
SET(SMACK_LABELING_SUPPORT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/wrt-client/process_pool/smack_labeling_support.cpp
    )
SET(SMACK_LABELING_SUPPORT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/wrt-client/process_pool/
   )
INCLUDE_DIRECTORIES(
    ${SMACK_LABELING_SUPPORT_INCLUDES}
    ${SMACK_LABELING_SUPPORT_DEPS_INCLUDE_DIRS}
    )
ADD_LIBRARY(${SMACK_LABELING_SUPPORT_STATIC} STATIC
    ${SMACK_LABELING_SUPPORT_SOURCES}
    )
TARGET_LINK_LIBRARIES(${SMACK_LABELING_SUPPORT_STATIC}
    ${SMACK_LABELING_SUPPORT_DEPS_LIBRARIES}
    )
# ------------------------------------------------------------------------------

#it is here, so no INCLUDE_DIRS and DEFINITIONS are passed there

SET(WRT_SRC_DIR
   ${PROJECT_SOURCE_DIR}/src
   )

SET(WRT_CORE_BASE_SOURCES
    ${WRT_SRC_DIR}/domain/application_data.cpp
    ${WRT_SRC_DIR}/domain/localization_setting.cpp
    ${WRT_SRC_DIR}/domain/main_thread.cpp
    ${WRT_SRC_DIR}/domain/permission_popup_manager.cpp
    ${WRT_SRC_DIR}/domain/widget_data_types.cpp
    ${WRT_SRC_DIR}/domain/widget_deserialize_model.cpp
    ${WRT_SRC_DIR}/domain/widget_model.cpp
    )

SET(WRT_CORE_INCLUDES
    ${WRT_SRC_DIR}/plugin-service
    ${WRT_SRC_DIR}/domain
    ${WRT_SRC_DIR}/profiling
    ${WRT_SRC_DIR}/view
    )

SET(WRT_PLUGIN_LOADING_DIR
    ${WRT_SRC_DIR}/plugin-service/plugin-loading/
   )


SET(PLUGIN_LOADING_INCLUDES
  ${WRT_PLUGIN_LOADING_DIR}
)

SET(WRT_BASIC_DEP
    dpl-efl
    dpl-dbus-efl
    dpl-event-efl
    dpl-utils-efl
    wrt-popup-ace-runner
    security-core
    security-client
)


SET(SYS_WRT_BASIC_DEP
    openssl
    libpcrecpp
    glib-2.0
    gthread-2.0
    libsoup-2.4
    libiri
    appsvc
    cert-svc-vcore
    edje
    ecore
    ecore-x
    evas
    eina
    ewebkit2
    elementary
    vconf
    cert-svc
    secure-storage
    capi-appfw-app-manager
)

IF(SMACK_ENABLED)
  LIST(APPEND SYS_WRT_BASIC_DEP libprivilege-control)
ENDIF(SMACK_ENABLED)



PKG_CHECK_MODULES(SYS_WRT_ENGINE_DEPS
    ${SYS_WRT_BASIC_DEP}
    REQUIRED
)

PKG_CHECK_MODULES(WRT_ENGINE_DEPS
    ${WRT_BASIC_DEP}
    REQUIRED
)

SET(WRT_ENGINE_INCLUDE_DIRS
    ${WRT_CORE_INCLUDES}
    ${PLUGIN_LOADING_INCLUDES}
    ${WRT_ENGINE_DEPS_INCLUDE_DIRS}
)

#wrt-engine-static
ADD_DEFINITIONS(${SYS_WRT_ENGINE_DEPS_CFLAGS})
ADD_DEFINITIONS(${SYS_WRT_ENGINE_DEPS_CFLAGS_OTHER})
ADD_DEFINITIONS(${WRT_ENGINE_DEPS_CFLAGS})
ADD_DEFINITIONS(${WRT_ENGINE_DEPS_CFLAGS_OTHER})

INCLUDE_DIRECTORIES(SYSTEM ${SYS_WRT_ENGINE_DEPS_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${WRT_ENGINE_INCLUDE_DIRS})

ADD_LIBRARY(${TARGET_WRT_ENGINE_STATIC} STATIC
    ${WRT_CORE_BASE_SOURCES}
)

SET_TARGET_PROPERTIES(${TARGET_WRT_ENGINE_STATIC} PROPERTIES
    SOVERSION ${PROJECT_API_VERSION}
    VERSION ${PROJECT_VERSION})

SET_TARGET_PROPERTIES(${TARGET_WRT_ENGINE_STATIC} PROPERTIES
    COMPILE_FLAGS -fPIC)
SET_TARGET_PROPERTIES(${TARGET_WRT_ENGINE_STATIC} PROPERTIES
    COMPILE_FLAGS "-include profiling_util.h")

INSTALL(FILES
    ${WRT_SRC_DIR}/domain/widget_model.h
    DESTINATION include/${PROJECT_NAME}
)

INSTALL(FILES
    ${WRT_SRC_DIR}/domain/widget_data_types.h
    DESTINATION include/${PROJECT_NAME}
)

ADD_SUBDIRECTORY(view)
ADD_SUBDIRECTORY(api_new)
ADD_SUBDIRECTORY(wrt-client)
ADD_SUBDIRECTORY(wrt-launcher)
ADD_SUBDIRECTORY(plugin-service)
IF(PROFILING)
    ADD_SUBDIRECTORY(profiling)
ENDIF(PROFILING)
ADD_SUBDIRECTORY(wrt-launchpad-daemon)
