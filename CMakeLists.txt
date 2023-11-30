############################################################################
# Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              #
#                                                                          #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-sqlite)

set(XEUS_SQLITE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XEUS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_FIND_FRAMEWORK NEVER)
set(CMAKE_FIND_APPBUNDLE NEVER)
set(CMAKE_PROGRAM_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Versioning
# ==========

file(STRINGS "${XEUS_SQLITE_INCLUDE_DIR}/xeus-sqlite/xeus_sqlite_config.hpp" xsqlite_version_defines
     REGEX "#define XSQLITE_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xsqlite_version_defines})
    if(ver MATCHES "#define XSQLITE_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+) *$")
        set(XSQLITE_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
${XSQLITE_VERSION_MAJOR}.${XSQLITE_VERSION_MINOR}.${XSQLITE_VERSION_PATCH})
message(STATUS "Building xeus-sqlite v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

# We generate the kernel.json file, given the installation prefix and the executable name
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xsqlite/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xsqlite/kernel.json"
)

# Build options
# =============

OPTION(XSQL_BUILD_STATIC "Build xeus-sqlite static library" ON)
OPTION(XSQL_BUILD_SHARED "Split xsqlite build into executable and library" ON)
OPTION(XSQL_BUILD_XSQLITE_EXECUTABLE "Build the xsqlite executable" ON)

OPTION(XSQL_USE_SHARED_XEUS "Link xsqlite with the xeus shared library (instead of the static library)" ON)
OPTION(XSQL_USE_SHARED_XEUS_SQLITE "Link xsqlite with the xeus-sqlite shared library (instead of the static library)" ON)

OPTION(XSQL_DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(XSQL_BUILD_TESTS "xeus-sqlite test suite" OFF)

if(EMSCRIPTEN)
    # for the emscripten build we need a FindSQLite3.cmake since
    # we install sqlite in a non-standart way
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_emscripten;${CMAKE_MODULE_PATH}")
endif()

# Dependencies
# ============

# Be sure to use recent versions (minimum requirements)
set(xvega_bindings_REQUIRED_VERSION 0.0.10)
set(tabulate_REQUIRED_VERSION 1.5)

find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(Threads REQUIRED)
find_package(xvega REQUIRED)
find_package(xvega-bindings ${xvega_bindings_REQUIRED_VERSION} REQUIRED)
find_package(tabulate ${tabulate_REQUIRED_VERSION} REQUIRED)

add_definitions(-DSQLITE_ENABLE_EXPLAIN_COMMENTS=1 -DSQLITE_DEBUG=1 -DSQLITE_MEMDEBUG=1)

# Target and link
# ===============

# xeus-sqlite source directory
set(XEUS_SQLITE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# xeus-sqlite source files
set(XEUS_SQLITE_SRC
    ${XEUS_SQLITE_SRC_DIR}/xeus_sqlite_interpreter.cpp
    ${XEUS_SQLITE_SRC_DIR}/xvega_sqlite.cpp
    ${XEUS_SQLITE_SRC_DIR}/xlite.cpp
)

set(XEUS_SQLITE_HEADERS
    include/xeus-sqlite/xeus_sqlite_config.hpp
    include/xeus-sqlite/xeus_sqlite_interpreter.hpp
    include/xeus-sqlite/xvega_sqlite.hpp
)

set(XSQLITE_SRC ${XEUS_SQLITE_SRC_DIR}/main.cpp)

# Targets and link - Macros
# =========================

include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

if(EMSCRIPTEN)
    add_compile_definitions(XSQL_EMSCRIPTEN_WASM_BUILD)
endif()

macro(xsql_set_common_options target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)
    
        # C++17 flag
        CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP17_FLAG)
        if (HAS_CPP17_FLAG)
            target_compile_features(${target_name} PRIVATE cxx_std_17)
        else ()
            message(FATAL_ERROR "Unsupported compiler -- xeus-sqlite requires C++17 support!")
        endif ()

        # Enable link time optimization and set the default symbol
        # visibility to hidden (very important to obtain small binaries)
        if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
            # Check for Link Time Optimization support
            # (GCC/Clang)
            CHECK_CXX_COMPILER_FLAG("-flto" HAS_LTO_FLAG)
            if (HAS_LTO_FLAG)
                target_compile_options(${target_name} PUBLIC -flto)
            endif ()

            # Avoids removing symbols from the static library
            CHECK_CXX_COMPILER_FLAG("-ffat-lto-objects" HAS_FATLTO_FLAG)
            if (${linkage_upper} MATCHES "STATIC" AND  HAS_FATLTO_FLAG)
                message(STATUS "ENABLE FAT LTO OBJECTS")
                target_compile_options(${target_name} PUBLIC -ffat-lto-objects)
            endif ()

            # Intel equivalent to LTO is called IPO
            if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
                CHECK_CXX_COMPILER_FLAG("-ipo" HAS_IPO_FLAG)
                if (HAS_IPO_FLAG)
                    target_compile_options(${target_name} PUBLIC -ipo)
                endif ()
            endif ()
        endif ()

    endif ()

    if (APPLE)
        set_target_properties(${target_name} PROPERTIES
            MACOSX_RPATH ON
        )
    else ()
        set_target_properties(${target_name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH 1
            SKIP_BUILD_RPATH FALSE
        )
    endif ()

    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endmacro()

# # Common macro kernels (xsqlite)
# macro(xsql_set_kernel_options target_name)

# Common macro for shared and static library xeus-sqlite
macro(xsql_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    add_library(${target_name} ${linkage_upper} ${XEUS_SQLITE_SRC} ${XEUS_SQLITE_HEADERS})
    xsql_set_common_options(${target_name})

    set_target_properties(${target_name} PROPERTIES
                          PUBLIC_HEADER "${XEUS_SQLITE_HEADERS}"
                          # COMPILE_DEFINITIONS "XEUS_SQLITE_EXPORTS"
                          PREFIX ""
                          VERSION ${${PROJECT_NAME}_VERSION}
                          SOVERSION ${XSQLITE_VERSION_MAJOR}
                          OUTPUT_NAME "lib${output_name}")

    target_compile_definitions(${target_name} PUBLIC "XEUS_SQLITE_EXPORTS")
    # target_compile_definitions(xsqlite PRIVATE XEUS_SQLITE_HOME="${XSQLITE_PREFIX}")

    target_include_directories(${target_name}
                               PUBLIC
                               ${XSQLITE_INCLUDE_DIRS}
                               $<BUILD_INTERFACE:${XEUS_SQLITE_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:include>)

    if (XSQL_USE_SHARED_XEUS)
        set(XSQL_XEUS_TARGET xeus)
    else ()
        set(XSQL_XEUS_TARGET xeus-static)
    endif ()

    message("XVEGA_STATIC_LIBRARY= ${XVEGA_STATIC_LIBRARY}")
    target_link_libraries(${target_name} PUBLIC 
      ${XSQL_XEUS_TARGET}
      ${XVEGA_STATIC_LIBRARY}
      SQLiteCpp
    )

    if(NOT EMSCRIPTEN)
        # find_package(Threads) # TODO: add Threads as a dependence of xeus-static?
        target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    endif()
endmacro()

# xeus-sqlite
# ===========

set(XEUS_SQLITE_TARGETS "")

if (XSQL_BUILD_SHARED)
    # Build libraries
    xsql_create_target(xeus-sqlite SHARED xeus-sqlite)
    list(APPEND XEUS_SQLITE_TARGETS xeus-sqlite)
endif ()

if (XSQL_BUILD_STATIC)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xsql_create_target(xeus-sqlite-static STATIC xeus-sqlite-static)
    else ()
        xsql_create_target(xeus-sqlite-static STATIC xeus-sqlite)
    endif ()
    list(APPEND XEUS_SQLITE_TARGETS xeus-sqlite-static)
endif ()

# xsqlite
# =======

if (XSQL_BUILD_XSQLITE_EXECUTABLE)
    find_package(xeus-zmq 1.1.1 REQUIRED)
    add_executable(xsqlite ${XSQLITE_SRC})
    set_target_properties(xsqlite PROPERTIES ENABLE_EXPORTS 1)
    target_link_libraries(xsqlite PRIVATE xeus-zmq)

    xsql_set_common_options(xsqlite)

    if (XSQL_USE_SHARED_XEUS_SQLITE)
        target_link_libraries(xsqlite PRIVATE xeus-sqlite)

        if(CMAKE_DL_LIBS)
            target_link_libraries(xsqlite PRIVATE ${CMAKE_DL_LIBS} util)
        endif()
    else ()
        target_link_libraries(xsqlite PRIVATE xeus-sqlite-static)
    endif()
endif()


# Tests
# =====

if(XSQL_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(XSQL_BUILD_TESTS ON)
endif()


if(XSQL_BUILD_TESTS)
    add_subdirectory(test)
endif()

if(EMSCRIPTEN)
    find_package(xeus-lite REQUIRED)
    include(WasmBuildOptions)
    add_executable(xsqlite src/main_emscripten_kernel.cpp )
    target_link_libraries(xsqlite PRIVATE xeus-sqlite-static xeus-lite)
    target_compile_features(xsqlite PRIVATE cxx_std_17)

    xeus_wasm_compile_options(xsqlite)
    xeus_wasm_link_options(xsqlite "web,worker")
    
    target_link_options(xsqlite
        PUBLIC "SHELL: -s FETCH=1"
        PUBLIC "SHELL: -s NO_EXIT_RUNTIME=1"
        PUBLIC "SHELL: -s 'ASYNCIFY_IMPORTS=[\"async_ems_init_idbfs\", \"async_ems_sync_db\"]'"
        PUBLIC "SHELL: -s FORCE_FILESYSTEM=1"
    )
    # target_link_libraries(xsqlite idbfs.js)
endif()

# Installation
# ============

include(CMakePackageConfigHelpers)

set(XEUS_SQLITE_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-sqliteConfig.cmake")

# Install xeus-sqlite and xsqlite
if (XSQL_BUILD_SHARED OR XSQL_BUILD_STATIC)
    install(TARGETS ${XEUS_SQLITE_TARGETS}
            EXPORT ${PROJECT_NAME}-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-sqlite)

    # Makes the project importable from the build directory
    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif ()

# Install xsqlite
if (XSQL_BUILD_XSQLITE_EXECUTABLE)
    install(TARGETS xsqlite
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (XSQL_BUILD_XSQLITE_EXECUTABLE OR EMSCRIPTEN)
    # Configuration and data directories for jupyter and xeus_sqlite
    set(XJUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

    # Install xsqlite Jupyter kernelspecs
    set(XSQL_KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
    install(DIRECTORY ${XSQL_KERNELSPEC_DIR}
            DESTINATION ${XJUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)


    # Extra path for installing Jupyter kernelspec
    if (XEXTRA_JUPYTER_DATA_DIR)
        install(DIRECTORY ${XSQL_KERNELSPEC_DIR}
                DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
                PATTERN "*.in" EXCLUDE)
    endif(XEXTRA_JUPYTER_DATA_DIR)
endif()

# Configure 'xeus-sqliteConfig.cmake' for a build tree
set(XEUS_SQLITE_CONFIG_CODE "####### Expanded from \@XEUS_SQLITE_CONFIG_CODE\@ #######\n")
set(XEUS_SQLITE_CONFIG_CODE "${XEUS_SQLITE_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_SQLITE_CONFIG_CODE "${XEUS_SQLITE_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-sqliteConfig.cmake' for an install tree
set(XEUS_SQLITE_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_SQLITE_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_SQLITE_CMAKECONFIG_INSTALL_DIR})

if (XSQL_BUILD_SHARED)
    install(EXPORT ${PROJECT_NAME}-targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION ${XEUS_SQLITE_CMAKECONFIG_INSTALL_DIR})
endif ()

if(EMSCRIPTEN)
    install(FILES
            "$<TARGET_FILE_DIR:xsqlite>/xsqlite.js"
            "$<TARGET_FILE_DIR:xsqlite>/xsqlite.wasm"
            DESTINATION ${XJUPYTER_DATA_DIR}/kernels/xsqlite)

endif()