# rv64gc_emu

RISCV emulator written in C++

## Features

- RV64IMAFDCSU fully implemented
- SV39/SV48/SV57 MMU
- SDL window that acts as a terminal emulator
    - Support for text mode: typing into SDL window will send data to the firmware over 16550 UART interface, and it will display text to the screen it received from the firmware over the UART interface.
    - Support for raw framebuffer mode: firmware can write to memory mapped memory locations to fill a framebuffer and display the buffer to the screen.
- PLIC
- CLINT
- bios (firmware), kernel and dtb loading
- Passes all [RISCV imafdcsu compliance tests](https://github.com/riscv-software-src/riscv-tests)

## Bulding

First, recursively pull this repo 

> git clone --recurse-submodules https://github.com/bane9/rv64gc-emu.git

Then, download the required tooling:

**Linux**

> sudo apt install cmake libsdl2-dev libsdl2-ttf-dev libicu-dev libvterm-dev

**Mac**

> brew install cmake sdl2 sdl2_ttf icu4c libvterm

In the directory of this git, type in the command line:

```
cmake . -Bbuild/ -DCMAKE_BUILD_TYPE=Release
cmake --build build/
```

The executable will be in the `build/rv64gc_emu`

## Usage
```
Usage: ./rv64gc_emu [options]
Options:
  -b, --bios bios_path     Path to the BIOS file (mandatory)
  -f, --font font_path     Path to the font file (mandatory)
  -d, --dtb dtb_path       Path to the device tree blob file (optional, mandatory if kernel is present)
  -k, --kernel kernel_path Path to the kernel file (optional)
```

`bios` option is meant either for baremetal firmware, or for a linux bootloader (e.g OpenSBI, BBL)

`font` will be used by the SDL window in text mode

`dtb` and `kernel` will be used to boot Linux

## Testing

To run the tests, first you need to build and install the [RISCV GNU Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)

After that, in the root directory of this git, type in your terminal (emulator needs to be built beforehand):

```
cmake -P MakeTests.cmake
cd build/
ctest
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

## License

This repository is under GNU GPLv3 license.

The RISC-V trade name is a registered trade mark of RISC-V International.