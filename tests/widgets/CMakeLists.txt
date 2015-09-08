# Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
# @author      Karol Pawlowski (k.pawlowski@samsung.com)
#

# TODO cleanup dependencies
PKG_CHECK_MODULES(COMMON_LIB_PKGS
    dbus-1
    libpcrecpp
    dpl-efl
    dpl-test-efl
    dpl-utils-efl
    dpl-wrt-dao-ro
    dpl-event-efl
    wrt-plugins-widgetdb
    glib-2.0
    gthread-2.0
    ewebkit2
    edje
    ecore
    ecore-x
    ecore-imf
    ecore-ipc
    ecore-evas
    ecore-file
    ecore-input
    evas
    eina
    elementary
    vconf
    aul
    libidn
    libiri
    REQUIRED
    )

INCLUDE(CMakeUtils.txt)

pkg_search_module(dpl REQUIRED dpl-efl)
pkg_search_module(dpl-test REQUIRED dpl-test-efl)

SET(WRT_TEST_LIBRARY "wrt-tests-libs")

include_directories(
  ${dpl_INCLUDE_DIRS}
  ${dpl-test_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${PROJECT_SOURCE_DIR}/src/view
  ${WRT_API_NEW_INCLUDES}
)

ADD_SUBDIRECTORY(files)
ADD_SUBDIRECTORY(common)

SET(TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TestInit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TestCases.cpp
)

SET(TESTS_TARGET "wrt-tests-general")

WRT_TEST_BUILD(${TESTS_TARGET} ${TESTS_SOURCES})
WRT_TEST_INSTALL(${TESTS_TARGET})
target_link_libraries(${TESTS_TARGET}
  ${dpl_LIBRARIES}
  ${dpl-test_LIBRARIES}
  ${WRT_TEST_LIBRARY}
  ${TARGET_CORE_MODULE_LIB}
  ${COMMON_LIB_PKGS_LIBRARIES}
)
