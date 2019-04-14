#include "viewer.hpp"

#include <iostream>

Viewer::~Viewer() {
  vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
  vkDestroyDevice(logicalDevice, nullptr);
  vkDestroySurfaceKHR(vkInstance, surface, nullptr);
  vkDestroyInstance(vkInstance, nullptr);

  glfwTerminate();
  glfwDestroyWindow(window);
}

void Viewer::run() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(width, height, "3D Viewer", nullptr, nullptr);

  glfwSetKeyCallback(window, exit_key_callback); 

  VkApplicationInfo applicationInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "3D Viewer",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "3D Viewer",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_1
  };

  auto glfwExtensionCount = uint32_t{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  VkInstanceCreateInfo instanceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo,
    .enabledExtensionCount = glfwExtensionCount,
    .ppEnabledExtensionNames = glfwExtensions
  };

  if (vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS) {
    throw std::runtime_error("Could not create vulkan instance");
  }

  if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("Could not create window surface");
  }

  auto pyhsicalDeviceCount = uint32_t{0};
  vkEnumeratePhysicalDevices(vkInstance, &pyhsicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(pyhsicalDeviceCount);
  vkEnumeratePhysicalDevices(vkInstance, &pyhsicalDeviceCount, physicalDevices.data());
  physicalDevice = physicalDevices[0];

  auto queuePriority = float{1.0f};
  VkDeviceQueueCreateInfo logicalDeviceQueueCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = 0,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority
  };

  VkPhysicalDeviceFeatures logicalDeviceFeatures{};
  VkDeviceCreateInfo logicalDeviceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &logicalDeviceQueueCreateInfo,
    .enabledExtensionCount = 1,
    .ppEnabledExtensionNames = logicalDeviceExtensions.data(),
    .pEnabledFeatures = &logicalDeviceFeatures
  };

  if (vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
    throw std::runtime_error("Could not create logical device");
  }

  vkGetDeviceQueue(logicalDevice, 0, 0, &graphicsQueue);
  vkGetDeviceQueue(logicalDevice, 0, 0, &presentationQueue);

  VkSurfaceFormatKHR surfaceFormat{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  VkPresentModeKHR presentationMode{VK_PRESENT_MODE_FIFO_KHR};
  VkExtent2D swapExtent{width, height};

  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

  VkSwapchainCreateInfoKHR swapChainCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = surface,
    .minImageCount = surfaceCapabilities.minImageCount + 1,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
    .imageExtent = swapExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = surfaceCapabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentationMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  if (vkCreateSwapchainKHR(logicalDevice, &swapChainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("Could not create swap chain");
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void Viewer::exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}
