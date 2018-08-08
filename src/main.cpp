#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

// ============================================================================

// The main handle which is used to store all per-application state values.
static VkInstance sInstance = NULL;

// ============================================================================

static void init_vulkan()
{
  // ==========================================================================
  // VkApplicationInfo - Structure specifying application information.
  // This is optional, but can be useful when debugging.
  //
  // Application and engine name and version can be used to given some nice
  // debugging information somewhere(?). API (apiVersion) is used to define the
  // highest supported Vulkan version. Patch version is being ignored when an
  // instance is being created.
  //
  // Some important notes from the Vulkan specs:
  // 1. sType must be VK_STRUCTURE_TYPE_APPLICATION_INFO.
  // 2. pNext must be NULL.
  // 3. pApplicationName is either NULL or a null terminated UTF-8 string.
  // ==========================================================================
  VkApplicationInfo applicationInfo;
  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  applicationInfo.pNext = NULL;
  applicationInfo.pApplicationName = "Vulkan Sandbox";
  applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.pEngineName = "Vulkan Sandbox Engine";
  applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  // ==========================================================================
  // VkInstanceCreateInfo - Structure specifying parameters for an instance.
  // This is mandatory.
  //
  // Flags (flags) are reserved for future use, so we can just ignore them now.
  // Application information can be null, but it may be used to somehow help
  // implementations recognize behavior inherent to classes of application.
  // Enabled layers and enabled extensions should contain either NULL's or ptrs
  // to arrays of null-terminated UTF-8 strings of the (layer/extension) names.
  //
  // Some important notes from the Vulkan specs:
  // 1. sType must be VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO.
  // 2. pNext must be NULL or pointer to valid Vk{extension}EXT structure.
  // 3. Each sType in the pNext chain must be unique.
  // 4. flags must be 0.
  // 5. pApplication must be NULL or a valid pointer to VkApplicationInfo.
  // 6. If enabledLayerCount != zero, ppEnabledLayerNames must be valid.
  // 7. If enabledExtensionCount != zero, ppEnabledExtensionNames must be valid.
  // ==========================================================================
  VkInstanceCreateInfo instanceInfo;
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pNext = NULL;
  instanceInfo.flags = 0;
  instanceInfo.pApplicationInfo = &applicationInfo;
  instanceInfo.enabledLayerCount = 0;
  instanceInfo.ppEnabledLayerNames = NULL;
  instanceInfo.enabledExtensionCount = 0;
  instanceInfo.ppEnabledExtensionNames = NULL;

  // ==========================================================================
  // vkCreateInstance - Create a new Vulkan instance.
  //
  // This function is used to create a new Vulkan instance. It automatically
  // checks whether requested layers and extensions exists and are supported.
  //
  // May return following kind of return codes:
  //   VK_SUCCESS                       When an instance was created.
  //   VK_ERROR_OUT_OF_HOST_MEMORY      When the host machine is out of memory.
  //   VK_ERROR_OUT_OF_DEVICE_MEMORY    When the device is out of memory.
  //   VK_ERROR_INITIALIZATION_FAILED   When the initialization failed.
  //   VK_ERROR_LAYER_NOT_PRESENT       If requested layer is not present.
  //   VK_ERROR_EXTENSION_NOT_PRESENT   If requested extension is not present.
  //   VK_ERROR_INCOMPATIBLE_DRIVER     If the device driver is incompatible.
  //
  // Some imporant notes from the Vulkan specs:
  // 1. pCreateInfo must be a pointer to VkInstanceCreateInfo structure.
  // 2. pAllocator must be NULL or a pointer to VkAllocationCallbacks.
  // 3. pInstance must be a pointer to a VkInstance handle.
  // ==========================================================================
  VkResult result = vkCreateInstance(&instanceInfo, NULL, &sInstance);
  switch (result) {
    case VK_SUCCESS:
      printf("vkCreateInstance succeeded.\n");
      break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      printf("vkCreateInstance failed: Host machine is out of memory.\n");
      break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      printf("vkCreateInstance failed: Device is out of memory.\n");
      break;
    case VK_ERROR_INITIALIZATION_FAILED:
      printf("vkCreateInstance failed: Vulkan initialization failed.\n");
      break;
    case VK_ERROR_LAYER_NOT_PRESENT:
      printf("vkCreateInstance failed: The requested layer is not present.\n");
      break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      printf("vkCreateInstance failed: The requested extension is not present.\n");
      break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      printf("vkCreateInstance failed: The graphics driver is incompatible.\n");
      break;
    default:
      printf("vkCreateInstance failed: An unknown error code [%d].\n", result);
      break;
  }
}

// ============================================================================

static void init()
{
  init_vulkan();
}

// ============================================================================

static void shutdown()
{
  if (sInstance != NULL) {
    vkDestroyInstance(sInstance, NULL);
    printf("vkDestroyInstance succeeded.");
  }
}

// ============================================================================

int main()
{
  atexit(shutdown);
  init();
  return 0;
}
