#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

const std::vector<const char*> validationLayers = {
  "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
  const bool enabledValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

// ============================================================================

// The main handle which is used to store all per-application state values.
static VkInstance sInstance = NULL;

// ============================================================================
// Get the result description for the specified Vulkan result code.
// @param result The target Vulkan result code.
// @returns A result description as a string.
static std::string vulkan_result_description(VkResult result)
{
  switch (result)
  {
    case VK_SUCCESS:
      return "Command successfully completed.";
    case VK_INCOMPLETE:
      return "A return array was too small for the result.";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "A host memory allocation has failed.";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "A device memory allocation has failed.";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "Initialization of an object could not be completed.";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "A requested layer is not present or could not be loaded.";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "A requested extension is not supported.";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "The requested version of Vulkan is not supported by the driver.";
    default:
      return "An unknown result code [" + std::to_string(result) + "] occured.";
  }
}

// ============================================================================

static void init_vulkan()
{
  // ==========================================================================
  // VALIDATION LAYERS
  // ==========================================================================
  // vkEnumerateInstanceLayerProperties - Enumerate Vulkan layers.
  // This is optional, but can be useful to check available layers.
  //
  // This feature is actually divided into two sections, where we must first
  // calculate the amount of available layers and the perform the actual query
  // to get information (name, description + version) about each layer.
  //
  // May return following kind of return codes:
  //  VK_SUCCESS
  //  VK_INCOMPLETE
  //  VK_ERROR_OUT_OF_HOST_MEMORY
  //  VK_ERROR_OUT_OF_DEVICE_MEMORY
  //
  // Some important notes from the Vulkan specs:
  // 1. pPropertyCount must be a valid uint32_t value.
  // 2. If pPropertyCount is not 0 then pProperties must be a valid array.
  // ==========================================================================

  // calculate the amount of available validation layers.
  uint32_t layerCount = 0;
  auto result = vkEnumerateInstanceLayerProperties(&layerCount, NULL);
  if (result != VK_SUCCESS) {
    printf("vkEnumerateInstanceLayerProperties failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }

  // gather information about the extensions.
  std::vector<VkLayerProperties> layers(layerCount);
  result = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
  if (result != VK_SUCCESS) {
    printf("vkEnumerateInstanceLayerProperties failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }

  // print out the list of supported extensions.
  printf("Found [%d] supported Vulkan layers(s):\n", layerCount);
  for (const auto& layer : layers) {
    printf("\t%s\n", layer.layerName);
  }

  // ==========================================================================
  // EXTENSIONS
  // ==========================================================================
  // vkEnumerateInstanceExtensionProperties - Enumerate Vulkan extensions.
  // This is optional, but can be useful to detect available extensions.
  //
  // This feature is actually divided into two sections, where we must first
  // calculate the amount of extensions on the current machine and then perform
  // the actual query to get information (name + version) about each extension.
  //
  // May return following kind of return codes:
  //   VK_SUCCESS                       When an instance was created.
  //   VK_INCOMPLETE                    When the pProperties array is too small.
  //   VK_ERROR_OUT_OF_HOST_MEMORY      When the host machine is out of memory.
  //   VK_ERROR_OUT_OF_DEVICE_MEMORY    When the device is out of memory.
  //   VK_ERROR_LAYER_NOT_PRESENT       If requested layer is not present.
  //
  // Some important notes from the Vulkan specs:
  // 1. If pLayerName is not NULL, playerName must be a null-terminated UTF-8.
  // 2. pPropertyCount must be a valid uint32_t value.
  // 3. If pPropertyCount is not 0 then pProperties must be a valid array.
  // ==========================================================================

  // calculate the amount of possible extensions.
  uint32_t extensionCount = 0;
  result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  if (result != VK_SUCCESS) {
    printf("vkEnumerateInstanceExtensionProperties failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }

  // gather information about the extensions.
  std::vector<VkExtensionProperties> extensions(extensionCount);
  result = vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions.data());
  if (result != VK_SUCCESS) {
    printf("vkEnumerateInstanceExtensionProperties failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }

  // print out the list of supported extensions.
  printf("Found [%d] supported Vulkan extension(s):\n", extensionCount);
  for (const auto& extension : extensions) {
    printf("\t%s\n", extension.extensionName);
  }

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
  if (enableValidationLayers) {
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    printf("Enabled [%d] validation layers:\n", instanceInfo.enabledLayerCount);
    for (const auto& validationLayer : validationLayers) {
      printf("\t%s\n", validationLayer);
    }
  } else {
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = NULL;
  }
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
  result = vkCreateInstance(&instanceInfo, NULL, &sInstance);
  if (result != VK_SUCCESS) {
    printf("vkCreateInstance failed: %s\n", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
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
