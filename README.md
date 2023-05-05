# RV64GC Emulator: A RISC-V Emulator developed in C++20

[![Build Status](https://github.com/bane9/rv64gc-emu/actions/workflows/build.yml/badge.svg)](https://github.com/bane9/rv64gc-emu/actions/workflows/build.yml)

Welcome to the RV64GC Emulator repository, a RISC-V emulator project developed using C++20, striving for high performance and comprehensive feature support. This emulator not only delivers a robust set of features but also provides Linux buildroot configuration and a port of DOOM, which can be found in the [rv64gc-emu-software](https://github.com/bane9/rv64gc-emu-software) repository.

## Table of Contents
- [Features](#features)
- [Building](#building)
- [Usage](#usage)
- [Testing](#testing)
- [Native CLI Option](#native-cli-option)
- [Emscripten Build Instructions](#emscripten-build-instructions)
- [Supported Platforms](#supported-platforms)
- [Dependencies/Credits](#dependenciescredits)
- [License](#license)

## Features

- RV64IMAFDCSU fully implemented
- SV39/SV48/SV57 MMU
- SDL window functioning as a terminal emulator
    - Text mode support: When typing into the SDL window, data is sent to the firmware through the 16550 UART interface. Moreover, the window fully supports and accurately displays received UART data, including properly handled ANSI escape sequences.
    - Raw framebuffer mode support: The firmware can write to memory-mapped locations to populate a framebuffer and display the contents on the screen.
- PLIC
- CLINT
- VIRTIO
- bios (firmware), kernel and dtb loading
- Successfully completes all [RISCV imafdcsu ISA tests](https://github.com/riscv-software-src/riscv-tests), with some caveats (see [Testing](#testing))

## Building

First, recursively clone this repo 

```bash
git clone --recurse-submodules https://github.com/bane9/rv64gc-emu.git
```

Then, download the required tooling:

**Linux**

```bash
sudo apt install cmake libsdl2-dev libsdl2-ttf-dev libicu-dev libvterm-dev
```

**Mac**

```bash
brew install cmake sdl2 sdl2_ttf icu4c libvterm
brew link icu4c --force
echo "ICU_ROOT=$(brew --prefix icu4c)" >> ~/.zshrc
```

In the directory of this git, type in the command line:

```bash
cmake . -Bbuild/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/
```

The executable will be in the `build/rv64gc_emu`

## Usage
```bash
Usage: ./rv64gc_emu [options]
Options:
  -b, --bios   Path to the BIOS file (mandatory)
  -f, --font   Path to the font file (mandatory)
  -d, --dtb    Path to the device tree blob file (optional, mandatory if kernel is present)
  -k, --kernel Path to the kernel file (optional)
  -m, --memory Emulator RAM buffer size in MiB (optional, default 64 MiB)
  -v, --virtual-drive Path to virtual disk image to use as a filesystem (optional)
```

`bios` option is meant either for bare-metal firmware, or for a linux bootloader (e.g OpenSBI, BBL, etc)

`font` will be used by the SDL window in text mode

`dtb` and `kernel` will be used to boot Linux

`memory` determines the amount of RAM the emulator should allocate. If the dtb argument is used, an additional 2MiB will be allocated, and the dtb will be stored in the top 2MiB.

`virtual-drive` path to a file that will be loaded to a virtio_blk device.

When a dtb is specified, the memory size register is expected to have the magic value `0x0badc0de`. For instance, the anticipated memory definitions in the DTS should appear as follows:

```dts
memory@80000000 {
  device_type = "memory";
  reg = <0x0 0x80000000 0x0 0x0badc0de>;
};
```

This magic value will be replaced either with the default RAM size value (66MiB) or with the specified memory size provided using the `--memory` argument. If the magic value is not present, ensure that you provide a memory size through the `--memory` argument that is equal to or larger than the size specified in the DTS.

## Testing

After building the emulator, install the riscv gnu toolchain:

**Linux**

```bash
sudo apt install gcc-riscv64-unknown-elf
```

**Mac**

```bash
brew tap riscv-software-src/riscv
brew install riscv-tools
```

Then, run these command from the root of this git:

```bash
cmake -P MakeTests.cmake
ctest --test-dir build/ --output-on-failure
```

Note: The RISCV F and D extensions use standardized FPU exceptions. This implementation depends on native FPU exceptions through the `fetestexcept` function to set the emulator's FPU exceptions. While this approach yields the expected results on M2 Mac devices, it doesn't produce the anticipated outcomes for the `FE_INEXACT` exception on other platforms. As a result, F and D ISA tests may not pass on your platform.

## Native CLI option

Should you need to eliminate the libvterm, sdl2, and icu4c dependencies, or if your platform is incompatible with them, consider compiling the emulator using the "Native CLI" flag. By doing so, the emulator will leverage the pre-existing terminal emulator for communication, bypassing the need for the aforementioned dependencies.

To do this, use this as the CMake build step:

```bash
cmake . -Bbuild/ -DCMAKE_BUILD_TYPE=Release -DNATIVE_CLI=1
cmake --build build/
```

In this case, the only dependency that needs to be acquired trough a package manager is `cmake`, and the usage of the program will be the following:

```bash
Usage: ./rv64gc_emu [options]
Options:
  -b, --bios   Path to the BIOS file (mandatory)
  -d, --dtb    Path to the device tree blob file (optional, mandatory if kernel is present)
  -k, --kernel Path to the kernel file (optional)
  -m, --memory Emulator RAM buffer size in MiB (optional, default 64 MiB)
```

Specyfing `-f, --font` will still be valid, but it will be ignored internally.

## Emscripten Build Instructions

1. Set up Emscripten by following the [official instructions](https://emscripten.org/docs/getting_started/downloads.html).

2. Open a terminal and navigate to the root of this git repository.

3. Create a new directory for the Emscripten build and move into it:

```bash
mkdir build_emscripten
cd build_emscripten
```

4. Run the following command to configure the Emscripten build:

```bash
emcmake cmake .. -DUSE_EMSCRIPTEN=ON -DEM_LOAD_PATH={Path to binaries} -DCMAKE_BUILD_TYPE=Release
```

Replace `{Path to binaries}` with the folder path containing your bios/firmware, kernel (if applicable), and dtb (if applicable). Note that the built Emscripten executable will exclusively have access to the contents of this directory and will not be able to read any other folders (subfolders are still accesible).

5. Build the program by typing:

```bash
make
```

6. To execute the program, run the following command:

```bash
node rv64gc_emu.js --bios ...
```

Please note that this will be built using the Native CLI configuration.

## Supported platforms

| Platform        | Compatible | Comments                           |
|-----------------|------------|------------------------------------|
| MacOS aarch64   | ✅         |                                    |
| MacOS x86       | ✅         | Currenly untested                  |
| Ubuntu x86      | ✅         | Currenly untested                  |
| Ubuntu aarch64  | ✅         |                                    |
| Emscripten      | ✅         | Native CLI only                    |

## Dependencies/credits

- [libvterm](http://www.leonerd.org.uk/code/libvterm/) - Downloaded via package manager
- [SDL2](https://www.libsdl.org/) - Downloaded via package manager
- [ICU4C](https://unicode-org.github.io/icu/userguide/icu4c/) - Downloaded via package manager
- [shimarin's vtermtest](https://gist.github.com/shimarin/71ace40e7443ed46387a477abf12ea70) - Included in source code (source/include/terminal.hpp)
- [fmtlib](https://github.com/fmtlib/fmt) - Included via CMake's FetchContent
- [MesloLGS NF Regular font](https://github.com/romkatv/dotfiles-public/blob/master/.local/share/fonts/NerdFonts/MesloLGS%20NF%20Regular.ttf) - included as font.ttf in the root of this git

## License

This repository is under GNU GPLv3 license.

The RISC-V trade name is a registered trade mark of RISC-V International.
