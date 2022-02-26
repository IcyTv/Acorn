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

### Dependencies

On Linux to install dependencies, just use the appropriate package manager.

Since Windows doesn't have a (good) package manager, you'll need to install some dependencies manually.

1. `Boost`. Look at the boost website for instructions of how to use their b2 utility to build or download the installer
    from <https://sourceforge.net/projects/boost/files/boost-binaries/1.78.0/>
    If you use the installer, because of a [meson bug](https://github.com/mesonbuild/meson/issues/8325), you will need to open
    the containing folder (meson produces an error could not read C:\\boost\\...\\*.pdb) and move the pdb files into a `pdb` folder.
    This makes sure that meson doesn't try parsing the pdb files.
2. `Depot Tools`. To enable v8 scripting in Acorn, use
   [this guide](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up) to setup depot_tools.
   Also make sure the environment variable `DEPOT_TOOLS_WIN_TOOLCHAIN` is set to `0`, in order to disallow gsync and gn from searching for googles distributed
   toolchains.

For the rest, meson will try to find the dependencies by using [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/) or [cmake](https://cmake.org/).
If it doesn't find any pre-installed version of the dependency, it will download them and add them to the buildchain automatically. To see what it downloaded, you
can look at the `subprojects` folder.

### Common Issues

If you are getting the error
> You must installWindows 10 SDK version 10.0.19041.0 including the "Debugging Tools for Windows" feature
while building v8, look [here](https://stackoverflow.com/questions/66710286/you-must-installwindows-10-sdk-version-10-0-19041-0-including-the-debugging-too)

## TODO's

This project is far from complete, and this is the place where I will keep track of some ideas I have for what to do. This is by no means a complete list, if you
have any ideas, feel free to open a new issue, or add to this list via a pull request. In the future we will likely move to an issue based system completely, but
because Acorn is so incomplete, I don't want to clutter the issue tracker.

- [ ] Setup Acorn to handle shared compilation on windows
- [ ] Move to mesons suggested folder structure (subprojects). For now we don't do this, since I hate how it organizes the project.
- [ ] Move the projects in Acorn/vendor to the meson wrap file system. Currently they don't have a meson.build file, therfore that doesn't work.

## Contributing

If you want to contribute, please read the [contributing guide](https://github.com/IcyTv/Acorn/tree/master/CONTRIBUTING.md)

## Copyright

Copyright 2022 Michael Finger <michael.finger@icytv.de>

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

> <http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
