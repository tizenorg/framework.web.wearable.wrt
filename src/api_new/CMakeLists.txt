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

ADD_LIBRARY(${TARGET_CORE_MODULE_LIB} SHARED
    ${PROJECT_SOURCE_DIR}/src/api_new/core_module.cpp
    ${PROJECT_SOURCE_DIR}/src/api_new/runnable_widget_object.cpp
    ${PROJECT_SOURCE_DIR}/src/api_new/runnable_widget_object_state.cpp
)

SET_TARGET_PROPERTIES(${TARGET_CORE_MODULE_LIB} PROPERTIES
#    COMPILE_FLAGS "-include profiling_util.h"
    OUTPUT_NAME ${TARGET_CORE_MODULE_LIB}
)

PKG_CHECK_MODULES(CORE_MODULE_DEP
    dpl-efl
    wrt-popup-wrt-runner
    REQUIRED
)

PKG_CHECK_MODULES(SYS_CORE_MODULE_DEP
    libprivilege-control
    cert-svc
    REQUIRED
)

INCLUDE_DIRECTORIES(
    ${CORE_MODULE_DEP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

INCLUDE_DIRECTORIES( SYSTEM ${SYS_CORE_MODULE_DEP_INCLUDE_DIRS})


#
#Set libraries that should be linked to the file that is linking wrt-core-module
#Do not set any static libraries as it will be included in wrt-core-module
#
SET(WRT_CORE_MODULE_INTERFACE_LIBRARIES
    ${WRT_ENGINE_DEPS_LIBRARIES}
    ${SYS_WRT_ENGINE_DEPS_LIBRARIES}
    ${CORE_MODULE_DEP_LIBRARIES}
    ${SYS_CORE_MODULE_DEP_LIBRARIES}
)

TARGET_LINK_LIBRARIES(${TARGET_CORE_MODULE_LIB}
# include all symbols that could be used by wrt-client or other binaries
    "-Wl,--whole-archive"
    ${TARGET_WRT_ENGINE_STATIC}
    ${TARGET_VIEW_COMMON_LIB_STATIC}
    "-Wl,--no-whole-archive"
    ${TARGET_VIEW_MODULE_LIB}
    ${WRT_CORE_MODULE_INTERFACE_LIBRARIES}
)

SET_TARGET_PROPERTIES(${TARGET_CORE_MODULE_LIB}
    PROPERTIES
    LINK_INTERFACE_LIBRARIES "${WRT_CORE_MODULE_INTERFACE_LIBRARIES}"
)

INSTALL(TARGETS ${TARGET_CORE_MODULE_LIB}
    DESTINATION lib/
    PERMISSIONS OWNER_READ OWNER_WRITE
    GROUP_READ GROUP_EXECUTE WORLD_READ
)

INSTALL(FILES
    ${PROJECT_SOURCE_DIR}/src/api_new/core_module.h
    ${PROJECT_SOURCE_DIR}/src/api_new/i_runnable_widget_object.h
    ${PROJECT_SOURCE_DIR}/src/api_new/user_delegates.h
    DESTINATION include/${PROJECT_NAME}
)
