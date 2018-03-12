// Copyright (c) 2014-2018 LG Electronics, Inc.
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
 * @file chargerlib.c
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include <nyx/nyx_module.h>
#include <nyx/module/nyx_utils.h>
#include <nyx/module/nyx_log.h>
#include "msgid.h"

nyx_device_t *nyxDev = NULL;
void *charger_status_callback_context = NULL;
void *state_change_callback_context = NULL;
nyx_device_callback_function_t charger_status_callback;
nyx_device_callback_function_t state_change_callback;

NYX_DECLARE_MODULE(NYX_DEVICE_CHARGER, "Main");

nyx_charger_status_t gChargerStatus =
{
	.charger_max_current = 0,
	.connected = 0,
	.powered = 0,
	.dock_serial_number = {0},
	.is_charging = true,
};

nyx_error_t nyx_module_open(nyx_instance_t i, nyx_device_t **d)
{
	if (NULL == d)
	{
		nyx_error(MSGID_NYX_QMUX_CHARG_OPEN_ERR, 0,"Charger device  open error.");
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
		nyx_error(MSGID_NYX_QMUX_CHARG_OUT_OF_MEMORY, 0, "Out of memory");
		return NYX_ERROR_OUT_OF_MEMORY;
	}

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_CHARGER_QUERY_CHARGER_STATUS_MODULE_METHOD,
	                           "charger_query_charger_status");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_CHARGER_REGISTER_CHARGER_STATUS_CALLBACK_MODULE_METHOD,
	                           "charger_register_charger_status_callback");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_CHARGER_ENABLE_CHARGING_MODULE_METHOD,
	                           "charger_enable_charging");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_CHARGER_DISABLE_CHARGING_MODULE_METHOD,
	                           "charger_disable_charging");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_CHARGER_REGISTER_STATE_CHANGE_CALLBACK_MODULE_METHOD,
	                           "charger_register_state_change_callback");

	nyx_module_register_method(i, (nyx_device_t *)nyxDev,
	                           NYX_CHARGER_QUERY_CHARGER_EVENT_MODULE_METHOD,
	                           "charger_query_charger_event");

	*d = (nyx_device_t *)nyxDev;
	return NYX_ERROR_NONE;
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

nyx_error_t charger_query_charger_status(nyx_device_handle_t handle,
        nyx_charger_status_t *status)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!status)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	memcpy(status, &gChargerStatus, sizeof(nyx_charger_status_t));
	return NYX_ERROR_NONE;
}

nyx_error_t charger_register_charger_status_callback(nyx_device_handle_t handle,
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

	charger_status_callback = callback_func;
	charger_status_callback_context = context;

	return NYX_ERROR_NONE;
}

nyx_error_t charger_enable_charging(nyx_device_handle_t handle,
                                    nyx_charger_status_t *status)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!status)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	memcpy(status, &gChargerStatus, sizeof(nyx_charger_status_t));
	return NYX_ERROR_NONE;
}

nyx_error_t charger_disable_charging(nyx_device_handle_t handle,
                                     nyx_charger_status_t *status)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!status)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	memcpy(status, &gChargerStatus, sizeof(nyx_charger_status_t));
	return NYX_ERROR_NONE;
}

nyx_error_t charger_register_state_change_callback(nyx_device_handle_t handle,
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

	state_change_callback = callback_func;
	state_change_callback_context = context;

	return NYX_ERROR_NONE;
}

nyx_error_t charger_query_charger_event(nyx_device_handle_t handle,
                                        nyx_charger_event_t *event)
{
	if (handle != nyxDev)
	{
		return NYX_ERROR_INVALID_HANDLE;
	}

	if (!event)
	{
		return NYX_ERROR_INVALID_VALUE;
	}

	*event = NYX_NO_NEW_EVENT;

	return NYX_ERROR_NONE;
}
