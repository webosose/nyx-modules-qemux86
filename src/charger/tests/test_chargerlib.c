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

#include <glib.h>
#include <stdio.h>

//
// Provide missing g_test macros if they are not defined in this version.
//
// We can't simply back-port the real definitions from glib as that would
// would change the license for this component.
//

#ifndef g_assert_true
#define g_assert_true(X) g_assert((X))
#endif

#ifndef g_assert_false
#define g_assert_false(X) g_assert(!(X))
#endif

#ifndef g_assert_nonnull
#define g_assert_nonnull(X) g_assert((X) != NULL)
#endif

#ifndef g_assert_null
#define g_assert_null(X) g_assert((X) == NULL)
#endif

//
// Pull in the relevant nyx headers. That way we can redefine macros
// if necessary (e.g. for logging) and the anti-recursion in the headers
// will let our redefinitions leak through into the UUT.
//
#include <nyx/nyx_module.h>
#include <nyx/module/nyx_utils.h>

//
// Mock out all the calls to nyx-lib
//
#undef nyx_info
#define nyx_info(m, args...) {}
#undef nyx_debug
#define nyx_debug(m, args...) {}
#undef nyx_error
#define nyx_error(m, args...) {}

// As we never actually call into nyx-lib, our nyx-lib instance
// can be anything we want it to be.
static nyx_instance_t the_instance = "an instance";

//
// Mock out the nyx call to register device methods
//
nyx_error_t nyx_module_register_method(nyx_instance_t instance,
                                       nyx_device_t *device_in_ptr,
                                       module_method_t method,
                                       const char *symbol_str)
{
	// Make sure the correct instance value gets passed through to nyx
	//
	// Could also add some tests to ensure only 'known' methods are
	// registered etc.
	g_assert_true(instance == the_instance);
	return NYX_ERROR_NONE;
}

//*****************************************************************************
//*****************************************************************************

// Pull in the unit under test
#include "../chargerlib.c"

//*****************************************************************************
//*****************************************************************************

// Mock the charger.c functions (from chargerlib.h)

static int32_t init_charger_max_current = -1;
static int32_t init_connected = -1;
static int32_t init_powered = -1;
static bool init_is_charging = true;
static char *init_serial_number = "serialNumber";
static char *new_serial_number = "newSerialNumber";

static void resetTestChargerStatus(nyx_charger_status_t *chargerStatus)
{
	g_assert_nonnull(chargerStatus);
	chargerStatus->charger_max_current = init_charger_max_current;
	chargerStatus->connected = init_connected;
	chargerStatus->powered = init_powered;
	chargerStatus->is_charging =
	    init_is_charging;  // no "bad" initial value for bool
	strncpy(chargerStatus->dock_serial_number, init_serial_number,
	        NYX_DOCK_SERIAL_NUMBER_LEN);
}

static void setTestChargerStatus(nyx_charger_status_t *chargerStatus)
{
	g_assert_nonnull(chargerStatus);
	chargerStatus->charger_max_current = 0;
	chargerStatus->connected = 0;
	chargerStatus->powered = 0;
	chargerStatus->is_charging = false; // opposite of initial value for bool
	strncpy(chargerStatus->dock_serial_number, new_serial_number,
	        NYX_DOCK_SERIAL_NUMBER_LEN);
}

nyx_error_t testChargerInit_retval = NYX_ERROR_NONE;
nyx_error_t _charger_init(void)
{
	return testChargerInit_retval;
}

nyx_error_t testChargerDeinit_retval = NYX_ERROR_NONE;
nyx_error_t _charger_deinit(void)
{
	return testChargerDeinit_retval;
}

nyx_error_t testChargerReadStatus_retval = NYX_ERROR_NONE;
nyx_error_t _charger_read_status(nyx_charger_status_t *status)
{
	g_assert_nonnull(status);

	// if we're not forcing an error, then set the return status fields
	if (NYX_ERROR_NONE == testChargerReadStatus_retval)
	{
		setTestChargerStatus(status);
	}

	return testChargerReadStatus_retval;
}

nyx_error_t testChargerEnableCharging_retval = NYX_ERROR_NONE;
nyx_error_t _charger_enable_charging(nyx_charger_status_t *status)
{
	g_assert_nonnull(status);

	// if we're not forcing an error, then change all of the return status fields
	// TODO: Should we set them to something interesting (to indicate charging enabled?)
	if (NYX_ERROR_NONE == testChargerEnableCharging_retval)
	{
		setTestChargerStatus(status);
	}

	return testChargerEnableCharging_retval;
}

nyx_error_t testChargerDisableCharging_retval = NYX_ERROR_NONE;
nyx_error_t _charger_disable_charging(nyx_charger_status_t *status)
{
	g_assert_nonnull(status);

	// if we're not forcing an error, then change all of the return status fields
	// TODO: Should we set them to something interesting (to indicate charging disabled?)
	if (NYX_ERROR_NONE == testChargerDisableCharging_retval)
	{
		setTestChargerStatus(status);
	}

	return testChargerDisableCharging_retval;
}

nyx_error_t testChargerQueryChargerEvent_retval = NYX_ERROR_NONE;
nyx_charger_event_t testChargerQueryChargerEvent_retEvent = NYX_NO_NEW_EVENT;
nyx_error_t _charger_query_charger_event(nyx_charger_event_t *event)
{
	g_assert_nonnull(event);
	*event = testChargerQueryChargerEvent_retEvent;
	return testChargerQueryChargerEvent_retval;
}

//*****************************************************************************
//*****************************************************************************


//
// All tests for API methods need an opened device (and need to close
// it afterwards), so they should use the following fixture, along with the
// setup and teardown functions.
//
// For ease, these tests can be added using the ADD_APITEST macro
//

typedef struct
{
	nyx_device_t *fixture_device;
} api_test_fixture;

//
// Setup for an API test by opening the module and storing the returned device
// in the fixture.
//
static void api_test_setup(api_test_fixture *fixture, gconstpointer unused)
{
	nyxDev = (nyx_device_t *) NULL;
	testChargerInit_retval = NYX_ERROR_NONE;
	fixture->fixture_device = NULL;
	g_assert_true(nyx_module_open(the_instance,
	                              &fixture->fixture_device) == NYX_ERROR_NONE);
	g_assert_nonnull(fixture->fixture_device);
}

//
//  Tear down an API test
//
static void api_test_teardown(api_test_fixture *fixture, gconstpointer unused)
{
	// Closing the module should never fail
	g_assert_true(nyx_module_close(fixture->fixture_device) == NYX_ERROR_NONE);
	fixture->fixture_device = NULL;
	nyxDev = (nyx_device_t *) NULL;
}

//
//  Add an API test using the fixture
//
#define ADD_APITEST(path, func) g_test_add(path, api_test_fixture, NULL, api_test_setup, func, api_test_teardown)

//*****************************************************************************
//*****************************************************************************

//
// Test the module-open method. Many of the tests are duplicated in the
// fixture setup and tear-down, but it's worth having a standalone test for
// the method.
//
static void test_module_open()
{
	nyx_device_t *test_device = NULL;

	// Force a failed open
	nyxDev = (nyx_device_t *) NULL;
	testChargerInit_retval = NYX_ERROR_GENERIC;
	test_device = NULL;
	g_assert_true(nyx_module_open(the_instance, &test_device) != NYX_ERROR_NONE);
	g_assert_null(test_device);

	// Check what should be a successful open
	nyxDev = (nyx_device_t *) NULL;
	testChargerInit_retval = NYX_ERROR_NONE;
	test_device = NULL;
	g_assert_true(nyx_module_open(the_instance, &test_device) == NYX_ERROR_NONE);
	g_assert_nonnull(test_device);

	// Check re-opening which should return an error
	nyx_device_t *dupe_device = NULL;
	g_assert_true(nyx_module_open(the_instance, &dupe_device) != NYX_ERROR_NONE);
	g_assert_null(dupe_device);

	// Release the device datastructure (and zero our pointer and theirs)
	free(test_device);
	test_device = NULL;
	nyxDev = NULL;
}


//
// Test for the charger_query_charger_status API
// nyx_error_t charger_query_charger_status(nyx_device_handle_t handle, nyx_charger_status_t *status)
//
static void test_charger_query_charger_status(api_test_fixture *fixture,
        gconstpointer unused)
{
	nyx_charger_status_t testChargerStatus;

	// Force a failed call
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_INVALID_HANDLE == charger_query_charger_status(NULL,
	              &testChargerStatus));

	// Force another failed call
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_INVALID_VALUE == charger_query_charger_status(
	                  fixture->fixture_device, NULL));

	// Check for no error
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_NONE == charger_query_charger_status(
	                  fixture->fixture_device, &testChargerStatus));

	// Check to make sure values returned have changed:
	g_assert_true(testChargerStatus.charger_max_current !=
	              init_charger_max_current);
	g_assert_true(testChargerStatus.connected != init_connected);
	g_assert_true(testChargerStatus.powered != init_powered);
	g_assert_true(testChargerStatus.is_charging != init_is_charging);
	g_assert_true(0 != strncmp(testChargerStatus.dock_serial_number,
	                           init_serial_number, NYX_DOCK_SERIAL_NUMBER_LEN));
}


//typedef void (*nyx_device_callback_function_t)(nyx_device_handle_t, nyx_callback_status_t, void *);
void test_nyx_device_callback_function(nyx_device_handle_t device,
                                       nyx_callback_status_t status, void *context)
{
	return;
}

//
// Test for the charger_register_charger_status_callback API
// nyx_error_t charger_register_charger_status_callback (nyx_device_handle_t handle, nyx_device_callback_function_t callback_func, void *context)
//
static void test_charger_register_charger_status_callback(
    api_test_fixture *fixture, gconstpointer unused)
{
	// Force a failed call
	g_assert_true(NYX_ERROR_INVALID_HANDLE ==
	              charger_register_charger_status_callback(NULL,
	                      test_nyx_device_callback_function, (void *)NULL));

	// Force another failed call
	g_assert_true(NYX_ERROR_INVALID_VALUE ==
	              charger_register_charger_status_callback(fixture->fixture_device, NULL,
	                      (void *)NULL));

	// Check for no error
	g_assert_true(NYX_ERROR_NONE == charger_register_charger_status_callback(
	                  fixture->fixture_device, test_nyx_device_callback_function, (void *)NULL));
}

//
// Test for the charger_enable_charging API
// nyx_error_t charger_enable_charging(nyx_device_handle_t handle, nyx_charger_status_t *status)
//
static void test_charger_enable_charging(api_test_fixture *fixture,
        gconstpointer unused)
{
	nyx_charger_status_t testChargerStatus;

	// Force a failed call
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_INVALID_HANDLE == charger_enable_charging(NULL,
	              &testChargerStatus));

	// Force another failed call
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_INVALID_VALUE == charger_enable_charging(
	                  fixture->fixture_device, NULL));

	// Check for no error
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_NONE == charger_enable_charging(fixture->fixture_device,
	              &testChargerStatus));

	// Check to make sure values returned have changed:
	g_assert_true(testChargerStatus.charger_max_current !=
	              init_charger_max_current);
	g_assert_true(testChargerStatus.connected != init_connected);
	g_assert_true(testChargerStatus.powered != init_powered);
	g_assert_true(testChargerStatus.is_charging != init_is_charging);
	g_assert_true(0 != strncmp(testChargerStatus.dock_serial_number,
	                           init_serial_number, NYX_DOCK_SERIAL_NUMBER_LEN));
}

//
// Test for the charger_disable_charging API
// nyx_error_t charger_disable_charging(nyx_device_handle_t handle, nyx_charger_status_t *status)
//
static void test_charger_disable_charging(api_test_fixture *fixture,
        gconstpointer unused)
{
	nyx_charger_status_t testChargerStatus;

	// Force a failed call
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_INVALID_HANDLE == charger_disable_charging(NULL,
	              &testChargerStatus));

	// Force another failed call
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_INVALID_VALUE == charger_disable_charging(
	                  fixture->fixture_device, NULL));

	// Check for no error
	resetTestChargerStatus(&testChargerStatus);
	g_assert_true(NYX_ERROR_NONE == charger_disable_charging(
	                  fixture->fixture_device, &testChargerStatus));

	// Check to make sure values returned have changed:
	g_assert_true(testChargerStatus.charger_max_current !=
	              init_charger_max_current);
	g_assert_true(testChargerStatus.connected != init_connected);
	g_assert_true(testChargerStatus.powered != init_powered);
	g_assert_true(testChargerStatus.is_charging != init_is_charging);
	g_assert_true(0 != strncmp(testChargerStatus.dock_serial_number,
	                           init_serial_number, NYX_DOCK_SERIAL_NUMBER_LEN));
}

//
// Test for the charger_register_state_change_callback API
// nyx_error_t charger_register_state_change_callback(nyx_device_handle_t handle, nyx_device_callback_function_t callback_func, void *context)
//
static void test_charger_register_state_change_callback(
    api_test_fixture *fixture, gconstpointer unused)
{
	// Force a failed call
	g_assert_true(NYX_ERROR_INVALID_HANDLE ==
	              charger_register_state_change_callback(NULL, test_nyx_device_callback_function,
	                      (void *)NULL));

	// Force another failed call
	g_assert_true(NYX_ERROR_INVALID_VALUE == charger_register_state_change_callback(
	                  fixture->fixture_device, NULL, (void *)NULL));

	// Check for no error
	g_assert_true(NYX_ERROR_NONE == charger_register_state_change_callback(
	                  fixture->fixture_device, test_nyx_device_callback_function, (void *)NULL));
}

//
// Test for the charger_query_charger_event API
// nyx_error_t charger_query_charger_event(nyx_device_handle_t handle, nyx_charger_event_t *event)
//
static void test_charger_query_charger_event(api_test_fixture *fixture,
        gconstpointer unused)
{
	nyx_charger_event_t testEvent = NYX_NO_NEW_EVENT;

	// Force a failed call
	g_assert_true(NYX_ERROR_INVALID_HANDLE == charger_query_charger_event(NULL,
	              &testEvent));

	// Force another failed call
	g_assert_true(NYX_ERROR_INVALID_VALUE == charger_query_charger_event(
	                  fixture->fixture_device, NULL));

	// Check for no error
	testEvent = NYX_NO_NEW_EVENT;
	testChargerQueryChargerEvent_retEvent = NYX_CHARGER_CONNECTED;
	g_assert_true(NYX_ERROR_NONE == charger_query_charger_event(
	                  fixture->fixture_device, &testEvent));
	g_assert_true(testEvent == NYX_CHARGER_CONNECTED);
}


//
// Set-up GLib, then register and run the tests.
int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/charger/api/module_open", test_module_open);
	ADD_APITEST("/charger/api/charger_query_charger_status",
	            test_charger_query_charger_status);
	ADD_APITEST("/charger/api/charger_register_charger_status_callback",
	            test_charger_register_charger_status_callback);
	ADD_APITEST("/charger/api/charger_enable_charging",
	            test_charger_enable_charging);
	ADD_APITEST("/charger/api/charger_disable_charging",
	            test_charger_disable_charging);
	ADD_APITEST("/charger/api/charger_register_state_change_callback",
	            test_charger_register_state_change_callback);
	ADD_APITEST("/charger/api/charger_query_charger_event",
	            test_charger_query_charger_event);

	return g_test_run();
}
