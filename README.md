# vulkan-sandbox
A sandbox to test and play around with Vulkan API.

## Prerequisities
This sandbox requires that the host machine has Vulkan SDK installed (>1.1.x.x).

Download Vulkan API from the LunarG website [here](https://vulkan.lunarg.com/sdk/home).

Check that the CFLAGS and LFLAGS in the Makefile point to the correct directory.

## Compiler
This project should be compiled with recent compilers, while older ones may not recognize and link with Vulkan libraries correctly. At least the g++ delivered with the latest MinGW package issued some linkage problems when I started with this project.
