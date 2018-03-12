nyx-modules-qemux86
===================

Summary
-------
Open webOS portability layer - emulator specific modules

Description
-----------

The nyx-modules-qemux86 repository is a collection of emulator specific modules for supporting different functionalities like charging, battery, keypad, touchscreen.

How to Build on Linux
=====================

## Dependencies

Below are the tools and libraries (and their minimum versions) required to build nyx-modules:

* cmake (version required by cmake-modules-webos)
* cmake-modules-webos 1.6.2
* gcc 4.8.4
* glib-2.0 2.32.1
* make (any version)
* nyx-lib 7.2.0
* pkg-config 0.26
* pmloglib 3.3.0

Below is the component (and its minimum version) required to run nyx-modules-qemux86:
* lsb 4.1

## Building

Once you have downloaded the source, enter the following to build it (after
changing into the directory under which it was downloaded):

    $ mkdir BUILD
    $ cd BUILD
    $ cmake -DNYX_MODULES_REQUIRED="NYXMOD_QEMU_BATTERY" ..
    $ make
    $ sudo make install

The directory under which the files are installed defaults to `/usr/local/webos`.
You can install them elsewhere by supplying a value for `WEBOS_INSTALL_ROOT`
when invoking `cmake`. You'll also need to tell cmake which Nyx modules to build
by supplying a semicolon-separated value for `NYX_MODULES_REQUIRED`. For example:

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/webosose -DNYX_MODULES_REQUIRED="NYXMOD_QEMU_BATTERY;NYXMOD_QEMU_CHARGER;NYXMOD_QEMU_KEYS;NYXMOD_QEMU_TOUCHPANEL" ..
    $ make
    $ make install

will install the files in subdirectories of `$HOME/projects/webosose`.

Specifying `WEBOS_INSTALL_ROOT` also causes `pkg-config` to look in that tree
first before searching the standard locations. You can specify additional
directories to be searched prior to this one by setting the `PKG_CONFIG_PATH`
environment variable.

If not specified, `WEBOS_INSTALL_ROOT` defaults to `/usr/local/webos`.

To configure for a debug build, enter:

    $ cmake -D CMAKE_BUILD_TYPE:STRING=Debug ..

To see a list of the make targets that `cmake` has generated, enter:

    $ make help

## Uninstalling

From the directory where you originally ran `make install`, enter:

 $ [sudo] make uninstall

You will need to use `sudo` if you did not specify `WEBOS_INSTALL_ROOT`.

# Copyright and License Information

Unless otherwise specified, all content, including all source code files and
documentation files in this repository are:

Copyright (c) 2010-2018 LG Electronics, Inc.

Unless otherwise specified or set forth in the NOTICE file, all content,
including all source code files and documentation files in this repository are:
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

SPDX-License-Identifier: Apache-2.0
