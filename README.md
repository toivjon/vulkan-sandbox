# vulkan-sandbox
A sandbox to test and play around with Vulkan API.

## Prerequisities
This sandbox requires that the host machine has Vulkan SDK installed (>1.1.x.x).

Download Vulkan API from the LunarG website [here](https://vulkan.lunarg.com/sdk/home).

Check that the CFLAGS and LFLAGS in the Makefile point to the correct directory.

## Compiler
Older older compilers may not recognize and link with Vulkan libraries correctly. At least the g++ (5.3.0) delivered with the latest MinGW package issued some linkage problems when I started with this project. Therefore Makefile uses Clang as the default compiler, where at least 6.0.1 version seems to work at the moment.
