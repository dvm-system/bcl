#
# Copyright 2018 Nikita Kataev
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.4.3)

PROJECT(BCL VERSION 3.0 LANGUAGES CXX)

set(BCL_DESCRIPTION "Base Construction Library")
set(BCL_AUTHOR "Nikita Kataev (kaniandr@gmail.com)")
set(BCL_HOMEPAGE_URL "https://bitbucket.org/dvm-system/idb")


option(BCL_LEGACY "Enable legacy code." OFF)
option(BCL_NODEJS_SOCKET "Enable Node.js wocket wrapper library." ON)

configure_file("include/bcl/bcl-config.h.in" "bcl/bcl-config.h")

add_library(Core INTERFACE)

# Populate core headers for GUI.
if(MSVC)
  target_sources(Core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/bcl/bcl-config.h>
    $<INSTALL_INTERFACE:include/bcl/bcl-config.h>)

  file(GLOB_RECURSE BCL_CORE_HEADERS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.h)
  foreach(F ${BCL_CORE_HEADERS})
    if(NOT BCL_LEGACY AND ${F} MATCHES "^include/bcl/legacy/.*")
      continue()
    endif()
    target_sources(Core INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${F}>
      $<INSTALL_INTERFACE:${F}>)
  endforeach()
endif(MSVC)

install(TARGETS Core EXPORT BCLExports)

set(BCL_INCLUDE_DIR
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(Core INTERFACE ${BCL_INCLUDE_DIR})

if (BCL_LEGACY)
  set(EXCLUDE_LEGACY "")
else()
  set(EXCLUDE_LEGACY "legacy")
endif()

install(DIRECTORY include/bcl DESTINATION include
  FILES_MATCHING PATTERN *.h PATTERN "${EXCLUDE_LEGACY}" EXCLUDE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bcl/bcl-config.h
  DESTINATION include/bcl)

# Build Config-file Package configuration.
export(EXPORT BCLExports
  FILE ${CMAKE_CURRENT_BINARY_DIR}/bcl/BCLExports.cmake NAMESPACE BCL::)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/bcl/BCLConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion)
configure_file(cmake/BCLConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/bcl/BCLConfig.cmake @ONLY)

# Copy all implemented CMake modules to binary directory.
file(GLOB_RECURSE BCL_CMAKE_MODULES "cmake/*.cmake")
foreach(F ${BCL_CMAKE_MODULES})
  configure_file(${F} ${CMAKE_CURRENT_BINARY_DIR}/bcl COPYONLY)
endforeach()

# Install Config-file Package configuration.
install(EXPORT BCLExports
  FILE BCLExports.cmake NAMESPACE BCL:: DESTINATION cmake)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/bcl/BCLConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/bcl/BCLConfigVersion.cmake
  DESTINATION cmake)
install(DIRECTORY cmake DESTINATION .
  FILES_MATCHING PATTERN *.cmake)

if (BCL_NODEJS_SOCKET)
  add_subdirectory(lib)
endif()

