###############################################################################
# Copyright (c) 2017, 2017 IBM Corp. and others
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at http://eclipse.org/legal/epl-2.0
# or the Apache License, Version 2.0 which accompanies this distribution
# and is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the
# Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
# version 2 with the GNU Classpath Exception [1] and GNU General Public
# License, version 2 with the OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] http://openjdk.java.net/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
###############################################################################

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(lvjit LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

### lua-omr
include_directories(.)
include_directories(lvjit)
include_directories(lua)
file(GLOB SRC_LUA lua/*.h lua/*.c)
file(GLOB SRC_LV_JIT lvjit/*.cpp)

get_filename_component(full_path_test_cpp lua/luac.c ABSOLUTE)
list(REMOVE_ITEM SRC_LUA "${full_path_test_cpp}")

get_filename_component(full_path_test_cpp lua/lua.c ABSOLUTE)
list(REMOVE_ITEM SRC_LUA "${full_path_test_cpp}")

add_library(lvjit
    ${SRC_LV_JIT}
    ${SRC_LUA})
#add_compile_definitions(LUA_C_LINKAGE)
target_compile_definitions(lvjit PUBLIC LUA_C_LINKAGE)
target_link_libraries(lvjit tril)
set_property(TARGET lvjit PROPERTY FOLDER fvtest/tril/examples)
