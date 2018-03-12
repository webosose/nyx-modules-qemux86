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

webos_build_nyx_module(BatteryMain
		       SOURCES batterylib.c
		       LIBRARIES ${GLIB2_LDFLAGS} ${PMLOG_LDFLAGS} ${NYXLIB_LDFLAGS} -lrt -lpthread)
add_subdirectory(tests)
install(FILES fake_battery_values.sh DESTINATION ${WEBOS_INSTALL_SBINDIR})
