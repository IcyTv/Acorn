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

- On Windows, you can use [vcpkg](https://vcpkg.io/) to install libraries and Visual Studio to build
- Linux Builds are not supported yet, but WILL come in the future

## TODO

- [ ] Implement ViewportChanged Event [[Viewport Change Event]]
- [ ] Implement basic Ultralight rendering
- [ ] Integrate with [svelte](#svelte)

## Svelte

If debug Mode:
 run svelte debug server and serve the website

If release mode:
 build svelte and serve local files
