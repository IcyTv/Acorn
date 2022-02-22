# Acorn

This is a game engine based on TheCherno's (Game Engine) Tutorial series.

## Requirements

### 0. (Windows) Visual Studio Setup

It's important to setup Visual Studio "correctly" in order to be able to build `v8`.

The Version of Visual Studio you need is either 2019 or 2017.
When installing, make sure you have the `Desktop Development with C++` workload selected.
Also make sure you install one of the "Windows SDK" components (Individual Components -> Windows SDK).

Now find the `Windows SDK` in "Apps & Features" (in the Windows settings) or "Programs and Features" (in the Control Panel).
Select Modify, then Change and next. Here you need to make sure `Debugging tools for Windows` is selected. Then finish the change.

### 1. Setup

>- Native Linux Builds are not supported yet, but WILL come in the future. The build system works, but we do not have
>  any backend to render. You can likely hack it, since linux supports glfw.

You will need at least `meson`, `ninja` and a [Compiler of your choice](#Compilers) installed.
If you want to enable scripting, you will also need `depot_tools` from google.

#### Compilers

If you are using Windows, install either Visual Studio (from 2017 on) or Visual Studio Build Tools.

## Contributing

If you want to contribute, please read the [contributing guide](https://github.com/IcyTv/Acorn/tree/master/CONTRIBUTING.md)

## Copyright

Copyright 2022 Michael Finger <michael.finger@icytv.de>

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

> <http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.