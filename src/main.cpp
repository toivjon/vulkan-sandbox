#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

int main()
{
  VkApplicationInfo applicationInfo = {};
  applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  applicationInfo.pNext = NULL;
  applicationInfo.pApplicationName = "Vulkan Sandbox";
  applicationInfo.pEngineName = "Vulkan Sandbox Engine";
  applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instanceInfo = {};
  instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pNext = NULL;
  instanceInfo.flags = 0;
  instanceInfo.pApplicationInfo = &applicationInfo;

  VkInstance instance;
  VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
  if (result != VK_SUCCESS) {
    printf("vkCreateInstance: Unable to initialize Vulkan instance.\n");
    exit(EXIT_FAILURE);
  }

  vkDestroyInstance(instance, NULL);

  return 0;
}
