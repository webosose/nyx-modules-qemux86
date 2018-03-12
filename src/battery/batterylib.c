// Copyright (c) 2010-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

/**
 * @file batterylib.c
 *
 * @brief Interface for components talking to the battery module using NYX api.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include <nyx/nyx_module.h>
#include <nyx/module/nyx_utils.h>
#include <nyx/module/nyx_log.h>
#include "msgid.h"

#define SYSFS_DEVICE "/tmp/powerd/fake/battery/"

#define   BATTERY_PERCENT   "getpercent"
#define   BATTERY_TEMPERATURE   "gettemp"
#define   BATTERY_VOLTS "getvoltage"
#define   BATTERY_CURRENT   "getcurrent"
#define   BATTERY_AVG_CURRENT   "getavgcurrent"
#define   BATTERY_FULL_40   "getfull40"
#define   BATTERY_RAW_COULOMB   "getrawcoulomb"
#define   BATTERY_COULOMB   "getcoulomb"
#define   BATTERY_AGE   "getage"

#define CHARGE_MIN_TEMPERATURE_C 0
#define CHARGE_MAX_TEMPERATURE_C 57
#define BATTERY_MAX_TEMPERATURE_C  60

nyx_device_t *nyxDev = NULL;
nyx_battery_ctia_t battery_ctia_params;
nyx_battery_status_t fake_battery_status;

void *battery_callback_context = NULL;
nyx_device_callback_function_t battery_callback = NULL;

NYX_DECLARE_MODULE(NYX_DEVICE_BATTERY, "Main");

int FileGetDouble(const char *path, double *ret_data)
{
	GError *gerror = NULL;
	char *contents = NULL;
	char *endptr;
	gsize len;
	float val;

	if (!path || !g_file_get_contents(path, &contents, &len, &gerror))
	{
		if (gerror)
		{
			nyx_error(MSGID_NYX_QMUX_BAT_GET_CONTENT_ERR, 0, "%s",gerror->message);
			g_error_free(gerror);
		}

		return -1;
	}

	val = strtod(contents, &endptr);

	if (endptr == contents)
	{
		nyx_error(MSGID_NYX_QMUX_BAT_STRTOD_ERR, 0, "Invalid input in %s.", path);
		goto end;
	}

	if (ret_data)
	{
		*ret_data = val;
	}

end:
	g_free(contents);
	return 0;
}

/**
 * @brief Read battery percentage
 *
 * @retval Battery percentage (integer)
 */
int battery_percent(void)
{
	int val;
	val = nyx_utils_read_value(SYSFS_DEVICE BATTERY_PERCENT);

	if (val < 0)
	{
		return -1;
	}

	return val;
}
/**
 * @brief Read battery temperature
 *
 * @retval Battery temperature (integer)
 */
int battery_temperature(void)
{
	int val;
	val = nyx_utils_read_value(SYSFS_DEVICE BATTERY_TEMPERATURE);

	if (val < 0)
	{
		return -1;
	}

	return val;
}

/**
 * @brief Read battery voltage
 *
 * @retval Battery voltage (integer)
 */

int battery_voltage(void)
{
	int val = 0;

	val = nyx_utils_read_value(SYSFS_DEVICE BATTERY_VOLTS);

	if (val < 0)
	{
		return -1;
	}

	/* Divide the value by 1000 to convert from uV to mV */
	return val / 1000;
}

/**
 * @brief Read the amount of current being drawn by the battery.
 *
 * @retval Current (integer)
 */
int battery_current(void)
{
	int val = 0;

	val = nyx_utils_read_value(SYSFS_DEVICE BATTERY_CURRENT);

	if (val < 0)
	{
		return -1;
	}

	/* Divide the value by 1000 to convert from uA to mA */
	return val / 1000;
}

/**
 * @brief Read average current being drawn by the battery.
 *
 * @retval Current (integer)
 */

int battery_avg_current(void)
{
	int val = 0;

	val = nyx_utils_read_value(SYSFS_DEVICE BATTERY_AVG_CURRENT);

	if (val < 0)
	{
		return -1;
	}

	/* Divide the value by 1000 to convert from uA to mA */
	return val / 1000;
}

/**
 * @brief Read battery full capacity
 *
 * @retval Battery capacity (double)
 */
double battery_full40(void)
{
	double val;

	if (FileGetDouble(SYSFS_DEVICE BATTERY_FULL_40, &val))
	{
		return -1;
	}

	return val;
}

/**
 * @brief Read battery current raw capacity
 *
 * @retval Battery capacity (double)
 */

double battery_rawcoulomb(void)
{
	double val;

	if (FileGetDouble(SYSFS_DEVICE BATTERY_RAW_COULOMB, &val))
	{
		return -1;
	}

	return val;
}

/**
 * @brief Read battery current capacity
 *
 * @retval Battery capacity (double)
 */

double battery_coulomb(void)
{
	double val;
	int ret;

	ret = FileGetDouble(SYSFS_DEVICE BATTERY_COULOMB, &val);

	if (ret)
	{
		return -1;
	}

	return val;
}

/**
 * @brief Read battery age
 *
 * @retval Battery age (double)
 */
double battery_age(void)
{
	double val;

	if (FileGetDouble(SYSFS_DEVICE BATTERY_AGE, &val))
	{
		return -1;
	}

	return val;
}


bool battery_is_present(void)
{
	int voltage = battery_voltage();
	return (voltage > 0);
}

nyx_error_t battery_init(void)
{
        int result = system("sh /usr/sbin/fake_battery_values.sh");
	if (result)
		return NYX_ERROR_GENERIC;

  	fake_battery_status.present = battery_is_present();

	if (fake_battery_status.present)
	{
		fake_battery_status.percentage = battery_percent();
		fake_battery_status.temperature = battery_temperature();
		fake_battery_status.voltage = battery_voltage();
		fake_battery_status.current = battery_current();
		fake_battery_status.avg_current = battery_avg_current();
		fake_battery_status.capacity = battery_coulomb();
		fake_battery_status.capacity_raw = battery_rawcoulomb();
		fake_battery_status.capacity_full40 = battery_full40();
		fake_battery_status.age = battery_age();

		if (fake_battery_status.avg_current >  0)
		{
			fake_battery_status.charging = true;
		}
	}
	else
	{
		fake_battery_status.charging = false;
	}
	return NYX_ERROR_NONE;
}

nyx_error_t nyx_module_open(nyx_instance_t i, nyx_device_t **d)
{
	if (NULL == d)
	{
		nyx_error(MSGID_NYX_QMUX_BAT_OPEN_ERR, 0,"Battery device  open error.");
		return NYX_ERROR_INVALID_VALUE;
	}

	*d = NULL;

	if (nyxDev)
	{
		return NYX_ERROR_TOO_MANY_OPENS;
	}

	nyxDev = (nyx_device_t *)calloc(sizeof(nyx_device_t), 1);

	if (NULL == nyxDev)
	{
		nyx_error(MSGID_NYX_QMUX_BAT_OUT_OF_MEM, 0,  "Out of memory");
		return NYX_ERROR_OUT_OF_MEMORY;
	}

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_QUERY_BATTERY_STATUS_MODULE_METHOD,
	                           "battery_query_battery_status");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_REGISTER_BATTERY_STATUS_CALLBACK_MODULE_METHOD,
	                           "battery_register_battery_status_callback");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_AUTHENTICATE_BATTERY_MODULE_METHOD,
	                           "battery_authenticate_battery");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_GET_CTIA_PARAMETERS_MODULE_METHOD,
	                           "battery_get_ctia_parameters");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_SET_WAKEUP_PARAMETERS_MODULE_METHOD,
	                           "battery_set_wakeup_percentage");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_SET_FAKE_MODE_MODULE_METHOD,
	                           "battery_set_fake_mode");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_BATTERY_GET_FAKE_MODE_MODULE_METHOD,
	                           "battery_get_fake_mode");

	nyx_error_t result = battery_init();

	if (NYX_ERROR_NONE != result)
	{
		free(nyxDev);
		nyxDev = NULL;
	}

	*d = (nyx_device_t *)nyxDev;
	return result;
}

nyx_error_t nyx_module_close(nyx_device_t *d)
{
	nyx_error_t result = NYX_ERROR_NONE;

	if (d == NULL)
	{
		result = NYX_ERROR_INVALID_VALUE;
	}

	if (NULL != nyxDev)
	{
		free(nyxDev);
		nyxDev = NULL;
	}

	return result;
}

nyx_battery_ctia_t *get_battery_ctia_params(void)
{
        battery_ctia_params.charge_min_temp_c = CHARGE_MIN_TEMPERATURE_C;
        battery_ctia_params.charge_max_temp_c = CHARGE_MAX_TEMPERATURE_C;
        battery_ctia_params.battery_crit_max_temp = BATTERY_MAX_TEMPERATURE_C;
        battery_ctia_params.skip_battery_authentication = true;

        return &battery_ctia_params;
}

nyx_error_t battery_query_battery_status(nyx_device_handle_t handle,
        nyx_battery_status_t *status)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!status)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	memcpy(status, &fake_battery_status, sizeof(nyx_battery_status_t));

	return NYX_ERROR_NONE;
}

nyx_error_t battery_register_battery_status_callback(nyx_device_handle_t handle,
        nyx_device_callback_function_t callback_func, void *context)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!callback_func)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	battery_callback_context = context;
	battery_callback = callback_func;

	return NYX_ERROR_NONE;
}

nyx_error_t battery_authenticate_battery(nyx_device_handle_t handle,
        bool *result)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}
	return NYX_ERROR_NONE;
}


nyx_error_t battery_get_ctia_parameters(nyx_device_handle_t handle,
                                        nyx_battery_ctia_t *param)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!param)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	nyx_battery_ctia_t *battery_ctia = get_battery_ctia_params();

	if (!battery_ctia)
	{
		return NYX_ERROR_INVALID_OPERATION;
	}

	memcpy(param, battery_ctia, sizeof(nyx_battery_ctia_t));
	return NYX_ERROR_NONE;
}

nyx_error_t battery_set_wakeup_percentage(nyx_device_handle_t handle,
        int percentage)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}
	return NYX_ERROR_NOT_IMPLEMENTED;
}

nyx_error_t battery_set_fake_mode(nyx_device_handle_t handle,
                                  bool enable)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}
	return NYX_ERROR_NOT_IMPLEMENTED;
}

nyx_error_t battery_get_fake_mode(nyx_device_handle_t handle,
                                  bool *enable)
{
	if (!enable)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

        *enable = false;
	return NYX_ERROR_NONE;
}
