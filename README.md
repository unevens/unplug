# Unplug

Unplug is a work in progress experiment to make VST3 plugins using [Dear ImGui](https://github.com/ocornut/imgui)
and [ImPlot](https://github.com/epezent/implot) instead of VSTGUI to do the user interface.

Unplug tries to deal internally with all the complexity of VST3 and native user interfaces, exposing a simple api so
that the user can just *unplug* from all the hassle typical of audio plugin development and just have fun.

Take a look at the `examples/gain` folder to see what that's look like, especially the files `UserInterface.cpp`
and `GainDsp.hpp`.

## Main features

- ready to use immediate-mode widgets that interact with the plugin parameters, and immediate-mode plots for showing
  audio waveforms and envelopes.
- lock-free data structures to share or resources between the audio thread and the user interface thread.
- template functions and helper classes to facilitate sample precise automation of the parameters.

## Getting started

Clone this repository with

```bash
$ git clone --recursive https://github.com/unevens/unplug
```

To build an example, got to its folder - they are located in the `examples` folder, and use the
provided `CMakeLists.txt` file, by either having your IDE loading it, or by doing something like

```bash
$ mkdir build
$ cd build
$ cmake ..
```

To create a new plugin, the easiest way is to copy an example and start from there. 

You can specify the correct information about the plugin and its author in the file `version.h`.

## Supported platforms

Currently, unplug works on Windows and Mac. It may work on Linux in the future, but that's not a priority for now.

Unplug can be built on Ubuntu, after installing these packages

```bash
$ sudo apt install libx11-dev
$ sudo apt install libgl1-mesa-dev
$ sudo apt install libxrandr-dev
$ sudo apt install libxcursor-dev
```

But there is no support for the user interface on Linux yet. If anyone who has some experience with X11 wants to
contribute to this, I would be glad to assist.

## Coming soon

- an example plugin with oversampling using [oversimple](https://github.com/unevens/oversimple)

## Credits

[Pugl](https://github.com/lv2/pugl) is used internally as a cross-platform window system.

===

VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
