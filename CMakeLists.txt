############################################################################
# Copyright (c) 2020, Mariana Meireles                                     #
# Copyright (c) 2020, QuantStack                                           #
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

# Versionning
# ===========

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

OPTION(XSQL_DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(XSQL_BUILD_TESTS "xeus-sqlite test suite" OFF)

# Dependencies
# ============

# Be sure to use recent versions
set(xeus_REQUIRED_VERSION 0.23.10)

find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(tabulate REQUIRED)
find_package(Threads REQUIRED)

add_definitions(-DSQLITE_ENABLE_EXPLAIN_COMMENTS=1 -DSQLITE_DEBUG=1 -DSQLITE_MEMDEBUG=1)

# Flags
# =====

include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP17_FLAG)

    if (HAS_CPP17_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
        set(CMAKE_CXX_STANDARD 17)
    else()
        message(FATAL_ERROR "Unsupported compiler -- xeus-sqlite requires C++17 support!")
    endif()
endif()

# Target and link
# ===============

# xeus-sqlite source directory
set(XEUS_SQLITE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# xeus-sqlite source files
set(XEUS_SQLITE_SRC
    ${XEUS_SQLITE_SRC_DIR}/xeus_sqlite_interpreter.cpp
)

set(XEUS_SQLITE_HEADERS
    include/xeus-sqlite/xeus_sqlite_interpreter.hpp
    include/xeus-sqlite/xeus_sqlite_config.hpp
)

# xeus-sqlite is the target for the library
add_library(xeus-sqlite SHARED ${XEUS_SQLITE_SRC} ${XEUS_SQLITE_HEADERS})


# xsqlite is the target for the kernel executable
add_executable(xsqlite src/main.cpp)
set_target_properties(xsqlite PROPERTIES ENABLE_EXPORTS 1)
target_link_libraries(xsqlite PRIVATE xeus-sqlite)

target_compile_definitions(xsqlite PRIVATE XEUS_SQLITE_HOME="${XSQLITE_PREFIX}")

if (APPLE)
    set_target_properties(xsqlite PROPERTIES
        MACOSX_RPATH ON
    )
else()
    set_target_properties(xsqlite PROPERTIES
        BUILD_WITH_INSTALL_RPATH 1
        SKIP_BUILD_RPATH FALSE
    )
endif()


set_target_properties(xsqlite xeus-sqlite PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_link_libraries(xeus-sqlite PUBLIC 
  xeus
  SQLiteCpp
  SQLite::SQLite3
)

set_target_properties(xsqlite PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_include_directories(xeus-sqlite
                           PUBLIC
                           ${XSQLITE_INCLUDE_DIRS}
                           $<BUILD_INTERFACE:${XEUS_SQLITE_INCLUDE_DIR}>
                           $<INSTALL_INTERFACE:include>)

set_target_properties(xeus-sqlite PROPERTIES
                      PUBLIC_HEADER "${XEUS_SQLITE_HEADERS}"
                      COMPILE_DEFINITIONS "XEUS_SQLITE_EXPORTS"
                      PREFIX ""
                      VERSION ${${PROJECT_NAME}_VERSION}
                      SOVERSION ${XSQLITE_VERSION_MAJOR}
                      OUTPUT_NAME "libxeus-sqlite")


# Tests
# =====

if(XSQL_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(XSQL_BUILD_TESTS ON)
endif()


if(XSQL_BUILD_TESTS)
    add_subdirectory(test)
endif()

# Installation
# ============

# Install xeus-sqlite and xsqlite
install(TARGETS xeus-sqlite
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-sqlite)

install(TARGETS xsqlite
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Configuration and data directories for jupyter and xeus_sqlite
set(XJUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

# Install Jupyter kernelspecs
set(XEUS_CALCSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
install(DIRECTORY ${XEUS_CALCSPEC_DIR}
        DESTINATION ${XJUPYTER_DATA_DIR}
        PATTERN "*.in" EXCLUDE)


# Extra path for installing Jupyter kernelspec
if (XEXTRA_JUPYTER_DATA_DIR)
    install(DIRECTORY ${XEUS_CALCSPEC_DIR}
            DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)
endif(XEXTRA_JUPYTER_DATA_DIR)
