# Copyright (c) 2010-2018 LG Electronics, Inc.
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

cmake_minimum_required(VERSION 2.8.7)

project(nyx-modules-qemux86)

include(webOS/webOS)
include(FindPkgConfig)

webos_modules_init(1 6 2)
webos_component(1 0 0)
webos_include_install_paths()

pkg_check_modules(PMLOG REQUIRED PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})

webos_nyx_module_provider(QEMU BATTERY CHARGER TOUCHPANEL KEYS)
webos_test_provider(GLIB_TEST)
webos_include_install_paths()

include_directories(include/internal)
add_subdirectory(src)

install(FILES files/conf/99-nyx-modules.rules DESTINATION ${WEBOS_INSTALL_SYSCONFDIR}/udev/rules.d)
