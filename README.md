# rv64gc Emulator

[![Build and Test](https://github.com/bane9/rv64gc-emu/actions/workflows/build_and_test.yml/badge.svg)](https://github.com/bane9/rv64gc-emu/actions/workflows/build_and_test.yml)

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
- Passes all [RISCV imafdcsu isa tests](https://github.com/riscv-software-src/riscv-tests) (at least on M2, on x86_64 they are failing)

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
  -b, --bios bios_path     Path to the BIOS file (mandatory)
  -f, --font font_path     Path to the font file (mandatory)
  -d, --dtb dtb_path       Path to the device tree blob file (optional, mandatory if kernel is present)
  -k, --kernel kernel_path Path to the kernel file (optional)
```

`bios` option is meant either for bare-metal firmware, or for a linux bootloader (e.g OpenSBI, BBL)

`font` will be used by the SDL window in text mode

`dtb` and `kernel` will be used to boot Linux

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
  -b, --bios bios_path     Path to the BIOS file (mandatory)
  -d, --dtb dtb_path       Path to the device tree blob file (optional, mandatory if kernel is present)
  -k, --kernel kernel_path Path to the kernel file (optional)
```

Specyfing `-f, --font` will still be valid, but it will be ignored internally.

## Supported platforms

| Platform      | Supported       | Comments                                      |
|---------------|-----------------|-----------------------------------------------|
| MacOS aarch64 | ✅               |                                               |
| MacOS x86     | ✅               | Currently not all riscv isa tests are passing |
| Ubuntu x86    | ✅               | Currently not all riscv isa tests are passing |
| Windows x86   | Native CLI only | Currenly untested                             |
| Emscripten    | ❌               | Planned future support                        |

## License

This repository is under GNU GPLv3 license.

The RISC-V trade name is a registered trade mark of RISC-V International.