# vulkan-sandbox
A sandbox to test and play around with Vulkan API.

## Prerequisities
This sandbox requires that the host machine has Vulkan SDK installed (>1.1.x.x).

Download Vulkan API from the LunarG website [here](https://vulkan.lunarg.com/sdk/home).

Check that the CFLAGS and LFLAGS in the Makefile point to the correct directory.

## Compiler
Some older or 32-bit compilers may not recognize and link with Vulkan libraries correctly.

At least g++ (ver. 8.1.0) delivered along with the MinGW-w64 seems to work at the moment.
