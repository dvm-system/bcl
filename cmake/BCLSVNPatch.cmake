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

# Applaies a patch 'patch' to a 'source_dir'
# Syntax: bcl_patch PATCHFILE patch TARGET source_dir
#         [REVERSE][IGNORE][PATCH][SUBVERSION]
#         [SUBVERSION_OPTIONS option ...]
#         [PATCH_OPTIONS option ...]
# If IGNORE is specified all errors will be treated as warrnings.
# SUBVERSION_OPTION and PATCH_OPTION can be used to specify options for
# `svn patch` and `patch` commands. Note, the `patch` command is the main
# command by default. The main command is executed at first and the
# second command is executed only if execution of a main command fails.
# If PATCH is specified then only `patch` is executed. If SUBVERSION is
# specified then svn patch is executed.
# Pre: patch command or subversion package (at least 1.7) must be available.
# Post: if `patch` and `source_dir` exists than the specified patch will be
# applied.
function(bcl_patch)
  cmake_parse_arguments(SP "REVERSE;IGNORE;PATCH;SUBVERSION"
    "PATCHFILE;TARGET" "SUBVERSION_OPTIONS;PATCH_OPTIONS" ${ARGN})
  if(NOT EXISTS ${SP_PATCHFILE} OR NOT EXISTS ${SP_TARGET})
    return()
  endif()
  if (SP_REVERSE)
    set(status "Reversing patch ${SP_PATCHFILE} to ${SP_TARGET}")
  else()
    set(status "Applying patch ${SP_PATCHFILE} to ${SP_TARGET}")
  endif()
  if (SP_IGNORE)
    set(FATAL_ERROR "WARNING")
    set(SEND_ERROR "WARNING")
  else()
    set(FATAL_ERROR "FATAL_ERROR")
    set(SEND_ERROR "SEND_ERROR")
  endif()
  message(STATUS ${status})
  if (NOT SP_PATCH AND SP_SUBVERSION)
    set(patch_not_exist true)
  else()
    execute_process(COMMAND patch -v OUTPUT_FILE /dev/null
      RESULT_VARIABLE patch_not_exist)
  endif()
  if (patch_not_exist)
    if (SP_PATCH AND NOT SP_SUBVERSION)
      message(${SEND_ERROR} "Patch command is not found (search for 'patch')")
      message(${FATAL_ERROR} "${status} - error")
    endif()
    find_package(Subversion "1.7") #svn patch appears in 1.7 version
    if(NOT Subversion_FOUND)
      message(${SEND_ERROR} "Patch command is not found (search for 'patch' or 'svn patch')")
      message(${FATAL_ERROR "${status} - error")
    endif()
    if(SP_REVERSE)
      set(options "--reverse-diff" ${SP_SUBVERSION_OPTIONS})
    else()
      set(options ${SP_SUBVERSION_OPTIONS})
    endif()
    execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} patch
      ${options} ${SP_PATCHFILE} ${SP_TARGET} RESULT_VARIABLE error)
  else()
    if(SP_REVERSE)
      set(REVERSE "-R")
    endif()
    execute_process(COMMAND patch ${SP_PATCH_OPTIONS} -Nstb ${REVERSE} -r /dev/null
      -d "${SP_TARGET}" -i ${SP_PATCHFILE} RESULT_VARIABLE error)
  endif()
  if(error)
    message(${FATAL_ERROR} "${status} - error")
  else()
    message(${STATUS} "${status} - done")
  endif()
endfunction(bcl_patch)
