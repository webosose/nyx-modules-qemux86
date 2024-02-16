# Copyright (c) 2010-2024 LG Electronics, Inc.
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
#
# SPDX-License-Identifier: Apache-2.0

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(NYXLIB REQUIRED nyx>=7)
include_directories(${NYXLIB_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${NYXLIB_CFLAGS_OTHER})

pkg_check_modules(PMLOG REQUIRED PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})

webos_add_linker_options(ALL --no-undefined --as-needed)
webos_add_compiler_flags(ALL -Wall -g -std=c99)
webos_add_compiler_flags(ALL -D_GNU_SOURCE=1)
webos_add_compiler_flags(DEBUG -O0 -DDEBUG -D_DEBUG)
webos_add_compiler_flags(RELEASE -DNDEBUG)

add_definitions(-DDEVICEINFO_PRODUCT_NAME="x86 Emulator")

if(NYXMOD_QEMU_BATTERY)
    add_subdirectory(battery)
endif()

if(NYXMOD_QEMU_CHARGER)
    add_subdirectory(charger)
endif()

if(NYXMOD_QEMU_TOUCHPANEL)
    add_subdirectory(touchpanel)
endif()

if(NYXMOD_QEMU_KEYS)
    add_subdirectory(keys)
endif()
