# rv64gc Emulator

[![Build](https://github.com/bane9/rv64gc-emu/actions/workflows/build.yml/badge.svg)](https://github.com/bane9/rv64gc-emu/actions/workflows/build.yml)

RISCV emulator written in C++20

## Table of Contents
- [Features](#features)
- [Building](#building)
- [Usage](#usage)
- [Testing](#testing)
- [Supported Software](#supported-software)
- [Dependencies/Credits](#dependenciescredits)
- [Native CLI Option](#native-cli-option)
- [Supported Platforms](#supported-platforms)
- [License](#license)

## Features

- RV64IMAFDCSU fully implemented
- SV39/SV48/SV57 MMU
- SDL window that acts as a terminal emulator
    - Support for text mode: typing into SDL window will send data to the firmware over 16550 UART interface, and it will display text to the screen it received from the firmware over the UART interface.
    - Support for raw framebuffer mode: firmware can write to memory mapped memory locations to fill a framebuffer and display the buffer to the screen.
- PLIC
- CLINT
- bios (firmware), kernel and dtb loading
- Successfully completes all [RISCV imafdcsu ISA tests](https://github.com/riscv-software-src/riscv-tests), with some caveats:
  - The RISCV F and D extensions use standardized FPU exceptions. This implementation depends on native FPU exceptions through the `fetestexcept` function to set the emulator's FPU exceptions. While this approach yields the expected results on M2 Mac devices, it doesn't produce the anticipated outcomes for the `FE_INEXACT` exception on other platforms. As a result, F and D ISA tests are disabled on Github Actions runs.

## Building

First, recursively pull this repo 

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
  -m, --memory Emulator RAM buffer size in MiB (optional, default 66 MiB)
```

`bios` option is meant either for bare-metal firmware, or for a linux bootloader (e.g OpenSBI, BBL)

`font` will be used by the SDL window in text mode

`dtb` and `kernel` will be used to boot Linux

`memory` determines the amount of RAM the emulator should allocate. If the dtb argument is used, an additional 2MiB will be allocated, and the dtb will be stored in the top 2MiB.

When a dtb is specified, the memory size register is expected to have the magic value `0x0badc0de`. For instance, the anticipated memory definitions in the DTS should appear as follows:

```dts
memory@80000000 {
  device_type = "memory";
  reg = <0x0 0x80000000 0x0 0x0badc0de>;
};
```

This magic value will be replaced either with the default RAM size value (66MiB) or with the specified memory size provided using the `--memory` argument. If the magic value is not present, ensure that you provide a memory size through the `--memory ` argument that is equal to or larger than the size specified in the DTS.

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

## Supported software

Linux buildroot configuration and a DOOM port can be found in the [rv64gc-emu-software](https://github.com/bane9/rv64gc-emu-software) git repository.

## Dependencies/credits

- [libvterm](http://www.leonerd.org.uk/code/libvterm/) - Downloaded via package manager
- [SDL2](https://www.libsdl.org/) - Downloaded via package manager
- [ICU4C](https://unicode-org.github.io/icu/userguide/icu4c/) - Downloaded via package manager
- [shimarin's vtermtest](https://gist.github.com/shimarin/71ace40e7443ed46387a477abf12ea70) - Included in source code (source/include/terminal.hpp)
- [fmtlib](https://github.com/fmtlib/fmt) - Included via CMake's FetchContent
- [MesloLGS NF Regular font](https://github.com/romkatv/dotfiles-public/blob/master/.local/share/fonts/NerdFonts/MesloLGS%20NF%20Regular.ttf) - included as font.ttf in the root of this git

## Native CLI option

In the case you want to drop the libvterm, sdl2 and icu4c dependencies (and if you are on a platform that doesn't support them), you can compile the emulator with the "Native CLI" flag where it would only use the existing terminal emulator for communication.

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

## Supported platforms

| Platform        | Supported        | Comments                                      |
|-----------------|------------------|-----------------------------------------------|
| MacOS aarch64   | ✅               |                                               |
| MacOS x86       | ✅               |                                               |
| Ubuntu x86      | ✅               |                                               |
| Ubuntu aarch64  | ✅               |                                               |
| Windows x86     | Native CLI only  | Currenly untested                             |
| Windows aarch64 | Native CLI only  | Currenly untested                             |
| Emscripten      | ❌               | Planned future support                        |

## License

This repository is under GNU GPLv3 license.

The RISC-V trade name is a registered trade mark of RISC-V International.