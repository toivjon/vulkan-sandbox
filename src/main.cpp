#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include <windows.h>

// TODO we actually should use vulkan.hpp instead?
#include <vulkan/vulkan.h>

const std::vector<const char*> VALIDATION_LAYERS = {
  "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> EXTENSIONS = {
  VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
  "VK_KHR_surface",
  "VK_KHR_win32_surface"
};

#ifdef NDEBUG
  const bool enabledValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

#define WINDOW_CLASS "window-class"

// ============================================================================

// The handle for the main window of the application.
static HWND sHWND = nullptr;

// The main handle which is used to store all per-application state values.
static VkInstance sInstance = VK_NULL_HANDLE;
// A handle that points to the selected physical graphics card.
static VkPhysicalDevice sPhysicalDevice = VK_NULL_HANDLE;
// The index of the selected physical device queue family.
static int sQueueFamilyIndex = 0;
// A handle that points to the created logical device.
static VkDevice sLogicalDevice = VK_NULL_HANDLE;
// A handle to created window surface.
static VkSurfaceKHR sSurface = VK_NULL_HANDLE;

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
// PHYSICAL DEVICES
// ============================================================================
// Vulkan seperates devices into physical and logical devices.
//
// Physical device represents a single complete implementation of Vulkan that
// is available for the host machine.
//
// Vulkan requires us to first enumerate and then select a suitable physical
// device for our application. For this purpose, we should do following things.
//
//   1. Enumerate available physical devices.
//   2. Check and rate devices based on their support to required properties.
//   3. Check and rate devices based on their support to required features.
//
// In addition to previously mentioned device support checks, we also need to
// check which queue family we can use in our processing. This is done by first
// enumerating all device queue families and then finding the index of a queue
// which supports our required set of features (e.g. graphics handling etc.).
//
// NOTE: Now we use a simple device selection, which does not score devices!
// ============================================================================

static std::vector<VkPhysicalDevice> enumerate_physical_devices()
{
  // calculate how many devices the host machine contains.
  uint32_t deviceCount = 0;
  auto result = vkEnumeratePhysicalDevices(sInstance, &deviceCount, NULL);
  if (result != VK_SUCCESS) {
    printf("vkEnumeratePhysicalDevices failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }

  // get handles for each available device.
  std::vector<VkPhysicalDevice> devices(deviceCount);
  result = vkEnumeratePhysicalDevices(sInstance, &deviceCount, devices.data());
  if (result != VK_SUCCESS) {
    printf("vkEnumeratePhysicalDevices failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }

  // return the results back to caller.
  printf("Vulkan API found [%d] physical device(s).\n", deviceCount);
  return devices;
}

// ============================================================================

static std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(const VkPhysicalDevice& device)
{
  // calculate how many queue families the physical device supports.
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

  // get handles for each available queue family.
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  // return the results back to caller.
  printf("Vulkan API found [%d] queue families for the target physical device.\n", queueFamilyCount);
  return queueFamilies;
}

// ============================================================================

static void select_vulkan_physical_device_and_queue_family()
{
  assert(sInstance != VK_NULL_HANDLE);
  printf("Selecting a physical device for Vulkan.\n");

  // iterate over the list of available physical devices.
  auto devices = enumerate_physical_devices();
  for (const auto& device : devices) {
    // get a support information from the device.
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceFeatures(device, &features);
    vkGetPhysicalDeviceProperties(device, &properties);

    // print out some support information.
    printf("\t%s\n", properties.deviceName);
    printf("\t\tsupports geometry shader:\t%d\n", features.geometryShader);
    printf("\t\tsupports tesselation shader:\t%d\n", features.tessellationShader);

    // check that the device has a queue family containing a support for graphics.
    auto queueFamilies = enumerate_queue_family_properties(device);
    for (auto i = 0u; i < queueFamilies.size(); i++) {
      // print out some queue family information.
      printf("\tqueue-family: %d\n", i);
      printf("\t\tsupports graphics:\t%d\n", (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 ? 1 : 0);
      printf("\t\tsupports compute:\t%d\n", (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 ? 1 : 0);

      // check whether we've found our device.
      bool supportsGraphics = (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
      if (features.geometryShader && features.tessellationShader && supportsGraphics) {
        sPhysicalDevice = device;
        sQueueFamilyIndex = i;
      }
    }
  }
}

// ============================================================================
// LOGICAL DEVICES
// ============================================================================
// Vulkan seperates devices into physical and logical devices.
//
// Logical device is used as an application specific interface for the physical
// interface, which is used to store application specific definitions in Vulkan.
//
// Same physical device can be used to create multiple logical devices. In our
// testing environment example, we now only create a single logical device.
// ============================================================================

static void create_logical_device()
{
  assert(sInstance != VK_NULL_HANDLE);
  assert(sPhysicalDevice != VK_NULL_HANDLE);

  // create a descriptor for the queues to be created for the device.
  float queuePriority = 1.f;
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.pNext = NULL;
  queueCreateInfo.flags = 0;
  queueCreateInfo.queueFamilyIndex = sQueueFamilyIndex;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  // specify a structure of which device features we will use.
  VkPhysicalDeviceFeatures deviceFeatures = {};

  // create a descriptor for a new logical device.
  VkDeviceCreateInfo createInfo;
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pNext = NULL;
  createInfo.flags = 0;
  createInfo.pQueueCreateInfos = &queueCreateInfo;
  createInfo.queueCreateInfoCount = 1;
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = 0;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;
  }

  // try to create the logical device with the descriptor.
  auto result = vkCreateDevice(sPhysicalDevice, &createInfo, nullptr, &sLogicalDevice);
  if (result != VK_SUCCESS) {
    printf("vkCreateDevice failed: %s", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }
  printf("Created a new Vulkan logical device for the application.\n");
}

// ============================================================================
// WINDOW SURFACES
// ============================================================================
//
// ============================================================================

static void create_window_surface()
{
  // create a descriptor to create a Vulkan window surface.
  VkWin32SurfaceCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.hwnd = sHWND;
  createInfo.hinstance = GetModuleHandle(nullptr);

  // explicitly load the necessary surface creation function.
  auto function = (PFN_vkCreateWin32SurfaceKHR) vkGetInstanceProcAddr(sInstance, "vkCreateWin32SurfaceKHR");
  if (!function) {
    printf("vkGetInstanceProc failed: unable to find vkCreateWin32SurfaceKHR.\n");
    exit(EXIT_FAILURE);
  }

  // try to create a new window surface.
  auto result = function(sInstance, &createInfo, nullptr, &sSurface);
  if (result != VK_SUCCESS) {
    printf("vkCreateWin32SurfaceKHR failed: %s\n", vulkan_result_description(result).c_str());
    exit(EXIT_FAILURE);
  }
  printf("Create a new window surface for the application.\n");
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
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
    instanceInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(EXTENSIONS.size());
    instanceInfo.ppEnabledExtensionNames = EXTENSIONS.data();
    printf("Enabled [%d] validation layers:\n", instanceInfo.enabledLayerCount);
    for (const auto& validationLayer : VALIDATION_LAYERS) {
      printf("\t%s\n", validationLayer);
    }
    printf("Enabled [%d] extensions:\n", instanceInfo.enabledExtensionCount);
    for (const auto& extension : EXTENSIONS) {
      printf("\t%s\n", extension);
    }
  } else {
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = NULL;
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = NULL;
  }

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

  select_vulkan_physical_device_and_queue_family();
  create_logical_device();
  create_window_surface();
}

// ============================================================================

static void shutdown()
{
  if (sInstance != NULL) {
    vkDestroyDevice(sLogicalDevice, NULL);
    vkDestroySurfaceKHR(sInstance, sSurface, NULL);
    vkDestroyInstance(sInstance, NULL);
    printf("vkDestroyInstance succeeded.");
  }
}

// ============================================================================
// WINAPI - Window process callback.
// ============================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_CLOSE:
      DestroyWindow(sHWND);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================================
// WINAPI - Unregister the application's window class.
// ============================================================================
static void unregister_window_class()
{
  UnregisterClass("window-class", GetModuleHandle(nullptr));
}

// ============================================================================
// WINAPI - Register a window class for the application.
// ============================================================================
static void register_window_class()
{
  // construct a descriptor a window class for our application.
  WNDCLASSEX wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize         = sizeof(WNDCLASSEX);
  wc.lpfnWndProc    = WndProc;
  wc.hInstance      = GetModuleHandle(nullptr);
  wc.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName  = WINDOW_CLASS;
  wc.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

  // try to register the new class.
  if (RegisterClassEx(&wc) == 0) {
    printf("RegisterClassEx: %ld\n", GetLastError());
    exit(-1);
  }
  atexit(unregister_window_class);
}

// ============================================================================
// WINAPI - Destruct the application's window.
// ============================================================================
static void destroy_window()
{
  DestroyWindow(sHWND);
}

// ============================================================================
// WINAPI - Create a window for the application.
// ============================================================================
static void create_window()
{
  // create a new window for our application.
  sHWND = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    "window-class",
    "Winapi - Sandbox",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    800, 600,
    NULL, NULL, GetModuleHandle(nullptr), NULL);
  if (sHWND == nullptr) {
    printf("CreateWindowEx: %ld\n", GetLastError());
    exit(EXIT_FAILURE);
  }
  atexit(destroy_window);
}

// ============================================================================

static void init_window()
{
  register_window_class();
  create_window();
}

// ============================================================================

static void init()
{
  init_window();
  init_vulkan();
}

// ============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  atexit(shutdown);
  init();

  MSG msg;
  ShowWindow(sHWND, nCmdShow);
  UpdateWindow(sHWND);
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  printf("%d %d %s %d\n", hInstance->unused, hPrevInstance->unused, lpCmdLine, nCmdShow);

  return 0;
}
