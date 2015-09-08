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
#
# @file        CMakeLists.txt
# @author      Przemyslaw Dobrowolski (p.dobrowolsk@samsung.com)
# @author      Pawel Sikorski (p.sikorski@samsung.com)
# @version     1.0
# @brief
#

# Check minimum CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Project name
PROJECT(wrt)

INCLUDE(FindPkgConfig)

STRING(REGEX MATCH "([^.]*)" PROJECT_API_VERSION "${PROJECT_VERSION}")
ADD_DEFINITIONS("-DPROJECT_API_VERSION=\"$(PROJECT_API_VERSION)\"")
MESSAGE(STATUS "Version from changelog: ${PROJECT_VERSION}")
#ADD_DEFINITIONS("-DWRT_SMACK_ENABLED")

#############################
# Build type
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

OPTION(DPL_LOG "DPL logs status" ON)
SET(LOG_TAG "WRT")
OPTION(WITH_TESTS "Build tests" OFF)
OPTION(CSP_SUPPORT "Support for csp policy" ON)
OPTION(ALLOW_NAVIGATION_SUPPORT "Support for allow-navigation" ON)
OPTION(APP_SCHEME_SUPPORT "Support for app:// scheme" OFF)
OPTION(CORS_WHITELISTING_SUPPORT "CORS white listing for access positions" OFF)
OPTION(ORIENTATION_SUPPORT "Support for screen orientation" OFF)
OPTION(JOURNAL_LOG_SUPPORT "Enable journal log" ON)

IF(JOURNAL_LOG_SUPPORT)
    ADD_DEFINITIONS("-DJOURNAL_LOG_SUPPORT")
ENDIF(JOURNAL_LOG_SUPPORT)

IF(CSP_SUPPORT)
    ADD_DEFINITIONS("-DCSP_ENABLED")
ENDIF(CSP_SUPPORT)
IF(ALLOW_NAVIGATION_SUPPORT)
    ADD_DEFINITIONS("-DALLOW_NAVIGATION_ENABLED")
ENDIF(ALLOW_NAVIGATION_SUPPORT)
IF(APP_SCHEME_SUPPORT)
    ADD_DEFINITIONS("-DAPP_SCHEME_ENABLED")
ENDIF(APP_SCHEME_SUPPORT)
IF(CORS_WHITELISTING_SUPPORT)
    ADD_DEFINITIONS("-DCORS_WHITELISTING_ENABLED")
ENDIF(CORS_WHITELISTING_SUPPORT)
IF(ORIENTATION_SUPPORT)
    ADD_DEFINITIONS("-DORIENTATION_ENABLED")
ENDIF(ORIENTATION_SUPPORT)

# logs can be only enabled in debug mode
IF(CMAKE_BUILD_TYPE MATCHES "Profiling" AND DPL_LOG)
    MESSAGE(STATUS "Logging disabled for DPL")
ELSE(CMAKE_BUILD_TYPE MATCHES "Profiling" AND DPL_LOG)
    ADD_DEFINITIONS("-DDPL_LOGS_ENABLED")
    MESSAGE(STATUS "Logging enabled for DPL")
ENDIF(CMAKE_BUILD_TYPE MATCHES "Profiling" AND DPL_LOG)

MESSAGE(STATUS "WITH_TESTS: " ${WITH_TESTS})
MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
OPTION(PROFILING "Profiling status" OFF)
IF(CMAKE_BUILD_TYPE MATCHES "Profiling")
    MESSAGE(STATUS "PROFILING: ENABLED")
    ADD_DEFINITIONS("-DPROFILING_ENABLED")
    SET(PROFILING ON)
ELSE(CMAKE_BUILD_TYPE MATCHES "Profiling")
    MESSAGE(STATUS "PROFILING: DISABLED")
ENDIF(CMAKE_BUILD_TYPE MATCHES "Profiling")

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build")
INCLUDE(Options)

# Compiler flags
SET(CMAKE_C_FLAGS_PROFILING    "-O2")
SET(CMAKE_CXX_FLAGS_PROFILING  "-O2 -std=c++0x")
SET(CMAKE_C_FLAGS_DEBUG        "-O0 -g")
SET(CMAKE_CXX_FLAGS_DEBUG      "-O0 -std=c++0x -g")
SET(CMAKE_C_FLAGS_RELEASE      "-Os")
SET(CMAKE_CXX_FLAGS_RELEASE    "-Os -std=c++0x -fvisibility-inlines-hidden")
SET(CMAKE_CXX_FLAGS_CCOV       "-O0 -std=c++0x -g --coverage")

ADD_DEFINITIONS("-fPIC")                        # If supported for the target machine, emit position-independent code, suitable for dynamic linking and avoiding any limit on the size of the global offset table. This option makes a difference on the m68k, PowerPC and SPARC. (BJ: our ARM too?)
#ADD_DEFINITIONS("-fvisibility=hidden")          # Set the default ELF image symbol visibility to hidden - all symbols will be marked with this unless overridden within the code.

# Set compiler warning flags
#ADD_DEFINITIONS("-Werror")                      # Make all warnings into errors.
ADD_DEFINITIONS("-Wall")                        # Generate all warnings
ADD_DEFINITIONS("-Wextra")                      # Generate even more extra warnings
ADD_DEFINITIONS("-Wno-variadic-macros")         # Inhibit variadic macros warnings (needed for ORM)
#ADD_DEFINITIONS("-ansi")                        # Accept only ANSI code
#ADD_DEFINITIONS("-pedantic")                    # Accept only pedantic code
#ADD_DEFINITIONS("-Weffc++")                     # Accept only effective C++ code
#ADD_DEFINITIONS("-Wwrite-strings")              # Do not accept writing to contant string memory
#ADD_DEFINITIONS("-Winit-self")                  # Do not accept initializing variable with itself
#ADD_DEFINITIONS("-Wcast-align")                 # Do not accept misaligning with casting
#ADD_DEFINITIONS("-Wcast-qual")                  # Do not accept removing qualifiers with casting
#ADD_DEFINITIONS("-Wold-style-cast")             # Do not accept old style casting
#ADD_DEFINITIONS("-Wpointer-arith")              # Warn about void pointer arthimetic
#ADD_DEFINITIONS("-Wstrict-aliasing")            # Ensure strict aliasing
#ADD_DEFINITIONS("-Wuninitialized")              # Do not accept uninitialized variables
#ADD_DEFINITIONS("-Wmissing-declarations")       # Warn about global and non-accesible functions
#ADD_DEFINITIONS("-Woverloaded-virtual")         # Warn about incidental overiding non-virtual base methods
#ADD_DEFINITIONS("-Wnon-virtual-dtor")           # Warn about non-virtual destructor
#ADD_DEFINITIONS("-Wctor-dtor-privacy")          # Warn about useless and non-constructible classes
#ADD_DEFINITIONS("-Wlong-long")                  # Do not allow using long long
#ADD_DEFINITIONS("-Wunreachable-code")           # Warn about unreachable code
#ADD_DEFINITIONS("-Wfloat-equal")                # Do not accept comparing floating points with equal operator
#ADD_DEFINITIONS("-Wabi")                        # Warn about possible ABI problems
#ADD_DEFINITIONS("-Wswitch-enum")                # Check switch enumeration
#ADD_DEFINITIONS("-Wformat=2")                   # Check printf formatting
#ADD_DEFINITIONS("-Wundef")                      # Warn if an undefined identifier is evaluated in an @if directive.
#ADD_DEFINITIONS("-Wshadow")                     # Warn whenever a local variable shadows another local variable, parameter or global variable or whenever a built-in function is shadowed
#ADD_DEFINITIONS("-Wconversion")                 # Warn for implicit conversions that may alter a value
#ADD_DEFINITIONS("-Wlogical-op")                 # Warn about suspicious uses of logical operators in expressions
#ADD_DEFINITIONS("-Waggregate-return")           # Warn if any functions that return structures or unions are defined or called.
#ADD_DEFINITIONS("-Wmissing-field-initializers") # Warn if a structure's initializer has some fields missing.
#ADD_DEFINITIONS("-Wredundant-decls")            # Warn if anything is declared more than once in the same scope, even in cases where multiple declaration is valid and changes nothing.
#ADD_DEFINITIONS("-Wmissing-include-dirs")       # Warn if a user-supplied include directory does not exist.
#ADD_DEFINITIONS("-Wswitch-default")             # Warn whenever a switch statement does not have a default case.
#ADD_DEFINITIONS("-Wsync-nand")                  # Warn when __sync_fetch_and_nand and __sync_nand_and_fetch built-in functions are used. These functions changed semantics in GCC 4.4.
#ADD_DEFINITIONS("-Wunused")                     # All the above -Wunused options combined.
#ADD_DEFINITIONS("-Wstrict-overflow=5")          # Also warn about cases where the compiler reduces the magnitude of a constant involved in a comparison.
#ADD_DEFINITIONS("-Wunsafe-loop-optimizations")  # Warn if the loop cannot be optimized because the compiler could not assume anything on the bounds of the loop indices. With -funsafe-loop-optimizations warn if the compiler made such assumptions.
#ADD_DEFINITIONS("-Wmissing-format-attribute")   # Warn about function pointers which might be candidates for format attributes.
#ADD_DEFINITIONS("-Wpadded")                     # Warn if padding is included in a structure, either to align an element of the structure or to align the whole structure.
#ADD_DEFINITIONS("-Winline")                     # Warn if a function can not be inlined and it was declared as inline.
#ADD_DEFINITIONS("-Wdisabled-optimization")      # Warn if a requested optimization pass is disabled.
ADD_DEFINITIONS("-Wno-deprecated")               # No warnings about deprecated features

#############################
# Targets names
SET(TARGET_WRT_ENGINE_STATIC  "wrt-engine-static")
SET(TARGET_TESTS_COMMON_LIB "wrt-engine-tests-lib") #tests only
SET(TARGET_PROFILING_LIB "wrt-profiling")
SET(TARGET_PLUGIN_MODULE_LIB "wrt-plugin-module")
SET(TARGET_VIEW_MODULE_LIB "wrt-view-module")
SET(TARGET_VIEW_COMMON_LIB_STATIC "wrt-view-common")
SET(TARGET_CORE_MODULE_LIB "wrt-core-module")
SET(TARGET_INJECTED_BUNDLE_LIB "wrt-injected-bundle")
SET(PROF_LIB "")
IF(PROFILING)
    SET(PROF_LIB ${TARGET_PROFILING_LIB})
ENDIF(PROFILING)

#############################
# definitions
ADD_DEFINITIONS("-DWRT_EDJ_PATH=\"/usr/share/edje/wrt/Wrt.edj\"")

#############################
# edc build
ADD_CUSTOM_COMMAND(
    OUTPUT ${PROJECT_SOURCE_DIR}/data/Wrt.edj
    COMMAND edje_cc
    ARGS  ${PROJECT_SOURCE_DIR}/data/Wrt.edc
          ${PROJECT_SOURCE_DIR}/data/Wrt.edj
    DEPENDS ${PROJECT_SOURCE_DIR}/data/Wrt.edc
)
ADD_CUSTOM_TARGET(WRT_EDJE ALL
    DEPENDS ${PROJECT_SOURCE_DIR}/data/Wrt.edj
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${PROJECT_SOURCE_DIR}/data/generic_popup.edj
    COMMAND edje_cc
    ARGS  ${PROJECT_SOURCE_DIR}/data/generic_popup.edc
          ${PROJECT_SOURCE_DIR}/data/generic_popup.edj
    DEPENDS ${PROJECT_SOURCE_DIR}/data/generic_popup.edc
)
ADD_CUSTOM_TARGET(WRT_GENERIC_POPUP_EDJE ALL
    DEPENDS ${PROJECT_SOURCE_DIR}/data/generic_popup.edj
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${PROJECT_SOURCE_DIR}/data/generic_popup_horizontal.edj
    COMMAND edje_cc
    ARGS  ${PROJECT_SOURCE_DIR}/data/generic_popup_horizontal.edc
          ${PROJECT_SOURCE_DIR}/data/generic_popup_horizontal.edj
    DEPENDS ${PROJECT_SOURCE_DIR}/data/generic_popup_horizontal.edc
)
ADD_CUSTOM_TARGET(WRT_GENERIC_POPUP_HORIZONTAL_EDJE ALL
    DEPENDS ${PROJECT_SOURCE_DIR}/data/generic_popup_horizontal.edj
)

#############################
# install
INSTALL(FILES
    ${PROJECT_SOURCE_DIR}/wrt_reset_db.sh
    ${PROJECT_SOURCE_DIR}/wrt_reset_all.sh
    DESTINATION /usr/bin/
)
INSTALL(FILES
    ${PROJECT_SOURCE_DIR}/data/Wrt.edj
    DESTINATION share/edje/wrt/
)
INSTALL(FILES
    ${PROJECT_SOURCE_DIR}/data/generic_popup.edj
    ${PROJECT_SOURCE_DIR}/data/generic_popup_horizontal.edj
    DESTINATION share/edje/ace/
)

#############################
# install launchpad files
INSTALL(FILES   ${CMAKE_SOURCE_DIR}/wrt_env.sh
    DESTINATION /etc/profile.d/
)
INSTALL(FILES   ${CMAKE_SOURCE_DIR}/wrt_launchpad_daemon.efl
    DESTINATION /etc/smack/accesses.d/
)
INSTALL(FILES   ${CMAKE_SOURCE_DIR}/wrt-launcher.efl
    DESTINATION /etc/smack/accesses.d/
)
INSTALL(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/systemd/wrt_launchpad_daemon.service
    DESTINATION /usr/lib/systemd/system/
)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(po)

SET(WRT_API_NEW_INCLUDES
    "${PROJECT_SOURCE_DIR}/src/api_new/"
    "${PROJECT_SOURCE_DIR}/src/domain/"
    "${PROJECT_SOURCE_DIR}/src/view/")

SET(PKGCONFIG_DIR ${PROJECT_SOURCE_DIR}/pkgconfig)

CONFIGURE_FILE(${PKGCONFIG_DIR}/wrt-core.pc.in
    ${PKGCONFIG_DIR}/wrt-core.pc @ONLY)
INSTALL(FILES ${PKGCONFIG_DIR}/wrt-core.pc DESTINATION lib/pkgconfig)

IF(WITH_TESTS)
    add_subdirectory(tests)
ENDIF(WITH_TESTS)
