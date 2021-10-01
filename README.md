# Game Engine

This is a game engine based on TheCherno's (Game Engine) Tutorial series.

## Requirements

- On Windows, you can use [vcpkg](https://vcpkg.io/) to install libraries and Visual Studio to build
- Linux Builds are not supported yet, but WILL come in the future

## TODO

- [ ] Figure Out vcpkg patching in order to remove build dependencies(?)
- [ ] Implement ViewportChanged Event [[Viewport Change Event]]
- [ ] Use [Overlay Ports](https://github.com/microsoft/vcpkg/blob/master/docs/users/manifests.md#vcpkg_overlay_triplets) to fix v8 build on windows
- [ ] Figure out snapshotting (currently is up to 20 seconds slower per script)
