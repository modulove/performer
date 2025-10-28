![Build Status](https://github.com/djphazer/performer/actions/workflows/ci.yml/badge.svg?branch=master)

# PEW|FORMER

_From [Phazerville](https://phazerville.com) with Love <3_

I'm working to bring this firmware up-to-date, and make myself at home with the UI controls. I've managed to update the toolchain to gcc 14.2, libopencm3 to the latest from October 2024, and the GitHub Actions CI workflow will produce an artifact file with the binaries for every push.

After encountering instability on the mebitek fork, I've decided to start from the known-good master branch of the original firmware, and carefully work in some of the new features from other forks.

## jackpf Improvements

| Change             	| Documentation                                    	|
|--------------------	|--------------------------------------------------	|
| Noise reduction    	| [docs](./doc/improvements/noise-reduction.md)    	|
| Shape improvements 	| [docs](./doc/improvements/shape-improvements.md) 	|
| MIDI improvements 	| [docs](./doc/improvements/midi-improvements.md) 	|

---

## 💙 A Heartfelt Thank You

First and foremost, a **huge thank you** to **westlicht** (Simon Kallweit) for creating the original PER|FORMER sequencer. This incredible instrument has been such an integral part of my modular system for years now! The generosity of open-sourcing this beast of a sequencer has enabled not just my own creative work, but an entire community of musicians and developers to push the boundaries of what's possible.

I hope with POW|FORMER I can do justice to your original vision and breathe some fresh life into this amazing platform while pushing the boundaries of what's possible with open source development. Thank you for creating something so special and sharing it with the world!

---

## POW|FORMER Features (modulove fork)

**Video Demo** - https://www.youtube.com/watch?v=F3FiG7CrAoI

This fork adds extensive performance-oriented features for live electronic music and modular integration:

### 🎹 16-Track Support
- **Dual bank system** - 2 banks of 8 tracks with visual LED distinction (bank 2 uses red LEDs)
- **Extended MIDI routing** - All 16 tracks available as note/gate/velocity/CC sources
- **Bank switching** - Seamless switching between banks on all pages
- **Limitations**: 8 patterns per track, 4 song slots (CONFIG_PATTERN_COUNT/CONFIG_SONG_SLOT_COUNT)

### 🌊 8 LFO Modulators
- **Independent modulators** - 8 LFOs with multiple waveform shapes:
  - Basic: Sine, Triangle, Saw Up/Down, Square
  - Random: Smooth Random, Stepped Random (with Smooth parameter 0-2000ms)
  - Gate-triggered: Gate Random Step
- **Waveform preview** - Real-time oscilloscope display
- **Flexible routing** - Quick-map popup for MIDI CC or Note output
- **Musical divisions** - Including triplets (1/4T, 1/8T, 1/16T, 1/32T, 1/64T) and dotted notes (1/2., 1/4., 1/8., 1/16., 1/32.)
- **Presets**:
  - "PR. T9-16": Maps tracks 9-16 to MIDI 1-8 @ velocity 127
  - "PR. M1-8": Maps Mod 1-8 to MIDI CC 0 on channels 1-8

### ⏱️ Microtiming Recording (v0.0.63)
- **7-bit resolution** - Gate offset range: -63 to +63 (127 values total)
- **Bidirectional timing** - Negative values trigger early, positive values delay
- **Capture Timing** - Record exact keyboard/MIDI input timing during live recording
- **Timing Quantize** (0-100%) - Blend between full microtiming capture and grid quantization
  - 0% = Full microtiming (natural feel)
  - 50% = Hybrid (default)
  - 100% = Full quantization (perfect grid)
- **High precision** - ~1ms per step @ 120 BPM
- **Live workflow** - Records timing automatically during performance

### 🎭 Enhanced Performer Page
- **Pattern mode LEDs**:
  - Green = Active (not muted)
  - Red = Muted
  - Yellow = Selected
- **Always-visible pattern numbers** - Dimmed display for muted tracks in pattern mode
- **All-tracks view** - See status of all 16 tracks simultaneously
- **Improved visual feedback** - Consistent LED behavior across both banks

### 🔌 Advanced MIDI/CV Routing
- **CV to MIDI CC** - Map CV inputs 1-4 to MIDI CC outputs
- **Extended sources** - Tracks 1-16, Mod 1-8, CV In 1-4 all available
- **Enum validation** - Crash protection for loading old projects
- **Improved display** - Clear source labels ("Mod 1-8", "CV In 1-4", "T9-16")

### 🎨 UI/UX Improvements (v0.0.63)
- **SVG-based startup logo** - Custom POW|FORMER logo with animated space invaders
- **New Keyboard page** - Dedicated 2-octave keyboard (14 white + 10 black keys) for live note input
- **Double-tap gate toggle** - Quick gate editing: double-tap any step in any layer to toggle gate on/off (300ms window)
- **Bank switching navigation** - Use Left/Right buttons to switch between track banks 1-8 and 9-16
- **LED color coding** - Bank 2 sequences use red LEDs for visual distinction
- **Shortened labels** - Optimized context menu text for readability
- **Consistent navigation** - Unified page behavior across all track modes
- **Better parameter editing** - Encoder acceleration and shift modifiers

### 💾 Technical Details (v0.0.63)
- **Memory footprint**: 361KB firmware (361028 text + 6660 data + 153404 bss)
- **Project compatibility**: Version27 format with backward compatibility
- **PPQN resolution**: 192 ticks per quarter note
- **Microtiming resolution**: 7-bit (-63 to +63), ~1ms per step @ 120 BPM

### 📦 Build Information
Firmware builds to `UPDATE.DAT` file ready for bootloader flashing. Latest build is automatically copied to Desktop as `UPDATE_16TRACKS_MINIMAL.DAT`.

--- original documentation below ---

<a href="doc/sequencer.jpg"><img src="doc/sequencer.jpg"/></a>

## Overview

This repository contains the firmware for the **PER|FORMER** eurorack sequencer.

For more information on the project go [here](https://westlicht.github.io/performer).

The hardware design files are hosted in a separate repository [here](https://github.com/westlicht/performer-hardware).

## Development

If you want to do development on the firmware, the following is a quick guide on how to setup the development environment to get you going.

### Setup on macOS and Linux

First you have to clone this repository (make sure to add the `--recursive` option to also clone all the submodules):

```
git clone --recursive https://github.com/westlicht/performer.git
```

After cloning, enter the performer directory:

```
cd performer
```

Make sure you have a recent version of CMake installed. If you are on Linux, you might also want to install a few other packages. For Debian based systems, use:

```
sudo apt-get install libtool autoconf cmake libusb-1.0.0-dev libftdi-dev pkg-config
```

To compile for the hardware and allow flashing firmware you have to install the ARM toolchain and build OpenOCD:

```
make tools_install
```

Next, you have to setup the build directories:

```
make setup_stm32
```

If you also want to compile/run the simulator use:

```
make setup_sim
```

The simulator is great when developing new features. It allows for a faster development cycle and a better debugging experience.

### Setup on Windows

Currently, there is no native support for compiling the firmware on Windows. As a workaround, there is a Vagrantfile to allow setting up a Vagrant virtual machine running Linux for compiling the application.

First you have to clone this repository (make sure to add the `--recursive` option to also clone all the submodules):

```
git clone --recursive https://github.com/westlicht/performer.git
```

Next, go to https://www.vagrantup.com/downloads.html and download the latest Vagrant release. Once installed, use the following to setup the Vagrant machine:

```
cd performer
vagrant up
```

This will take a while. When finished, you have a virtual machine ready to go. To open a shell, use the following:

```
vagrant ssh
```

When logged in, you can follow the development instructions below, everything is now just the same as with a native development environment on macOS or Linux. The only difference is that while you have access to all the source code on your local machine, you use the virtual machine for compiling the source.

To stop the virtual machine, log out of the shell and use:

```
vagrant halt
```

You can also remove the virtual machine using:

```
vagrant destroy
```

### Build directories

After successfully setting up the development environment you should now have a list of build directories found under `build/[stm32|sim]/[release|debug]`. The `release` targets are used for compiling releases (more code optimization, smaller binaries) whereas the `debug` targets are used for compiling debug releases (less code optimization, larger binaries, better debugging support).

### Developing for the hardware

You will typically use the `release` target when building for the hardware. So you first have to enter the release build directory:

```
cd build/stm32/release
```

To compile everything, simply use:

```
make -j
```

Using the `-j` option is generally a good idea as it enables parallel building for faster build times.

To compile individual applications, use the following make targets:

- `make -j sequencer` - Main sequencer application
- `make -j sequencer_standalone` - Main sequencer application running without bootloader
- `make -j bootloader` - Bootloader
- `make -j tester` - Hardware tester application
- `make -j tester_standalone` - Hardware tester application running without bootloader

Building a target generates a list of files. For example, after building the sequencer application you should find the following files in the `src/apps/sequencer` directory relative to the build directory:

- `sequencer` - ELF binary (containing debug symbols)
- `sequencer.bin` - Raw binary
- `sequencer.hex` - Intel HEX file (for flashing)
- `sequencer.srec` - Motorola SREC file (for flashing)
- `sequencer.list` - List file containing full disassembly
- `sequencer.map` - Map file containing section/offset information of each symbol
- `sequencer.size` - Size file containing size of each section

If compiling the sequencer, an additional `UPDATE.DAT` file is generated which can be used for flashing the firmware using the bootloader.

To simplify flashing an application to the hardware during development, each application has an associated `flash` target. For example, to flash the bootloader followed by the sequencer application use:

```
make -j flash_bootloader
make -j flash_sequencer
```

Flashing to the hardware is done using OpenOCD. By default, this expects an Olimex ARM-USB-OCD-H JTAG to be attached to the USB port. You can easily reconfigure this to use a different JTAG by editing the `OPENOCD_INTERFACE` variable in the `src/platform/stm32/CMakeLists.txt` file. Make sure to change both occurrences. A list of available interfaces can be found in the `tools/openocd/share/openocd/scripts/interface` directory (or `/home/vagrant/tools/openocd/share/openocd/scripts/interface` when running the virtual machine).

### Developing for the simulator

Note that the simulator is only supported on macOS and Linux and does not currently run in the virtual machine required on Windows.

You will typically use the `debug` target when building for the simulator. So you first have to enter the debug build directory:

```
cd build/sim/debug
```

To compile everything, simply use:

```
make -j
```

To run the simulator, use the following:

```
./src/apps/sequencer/sequencer
```

Note that you have to start the simulator from the build directory in order for it to find all the assets.

### Source code directory structure

The following is a quick overview of the source code directory structure:

- `src` - Top level source directory
- `src/apps` - Applications
- `src/apps/bootloader` - Bootloader application
- `src/apps/hwconfig` - Hardware configuration files
- `src/apps/sequencer` - Main sequencer application
- `src/apps/tester` - Hardware tester application
- `src/core` - Core library used by both the sequencer and hardware tester application
- `src/libs` - Third party libraries
- `src/os` - Shared OS helpers
- `src/platform` - Platform abstractions
- `src/platform/sim` - Simulator platform
- `src/platform/stm32` - STM32 platform
- `src/test` - Test infrastructure
- `src/tests` - Unit and integration tests

The two platforms both have a common subdirectories:

- `drivers` - Device drivers
- `libs` - Third party libraries
- `os` - OS abstraction layer
- `test` - Test runners

The main sequencer application has the following structure:

- `asteroids` - Asteroids game
- `engine` - Engine responsible for running the sequencer core
- `model` - Data model storing the live state of the sequencer and many methods to change that state
- `python` - Python bindings for running tests using python
- `tests` - Python based tests
- `ui` - User interface

## Third Party Libraries

The following third party libraries are used in this project.

- [FreeRTOS](http://www.freertos.org)
- [libopencm3](https://github.com/libopencm3/libopencm3)
- [libusbhost](https://github.com/libusbhost/libusbhost)
- [NanoVG](https://github.com/memononen/nanovg)
- [FatFs](http://elm-chan.org/fsw/ff/00index_e.html)
- [stb_sprintf](https://github.com/nothings/stb/blob/master/stb_sprintf.h)
- [stb_image_write](https://github.com/nothings/stb/blob/master/stb_image_write.h)
- [soloud](https://sol.gfxile.net/soloud/)
- [RtMidi](https://www.music.mcgill.ca/~gary/rtmidi/)
- [pybind11](https://github.com/pybind/pybind11)
- [tinyformat](https://github.com/c42f/tinyformat)
- [args](https://github.com/Taywee/args)

## License

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

This work is licensed under a [MIT License](https://opensource.org/licenses/MIT).
