#include "viewer.hpp"

#include <iostream>
#include <vector>

Viewer::~Viewer() {
  vkDestroyDevice(logicalDevice, nullptr);
  vkDestroyInstance(vkInstance, nullptr);

  glfwTerminate();
  glfwDestroyWindow(window);
}

void Viewer::run() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(640, 480, "3D Viewer", nullptr, nullptr);

  glfwSetKeyCallback(window, exit_key_callback); 

  VkApplicationInfo applicationInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "3D Viewer",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "3D Viewer",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_1
  };

  auto glfwExtensionCount = uint32_t{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  VkInstanceCreateInfo instanceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo,
    .enabledExtensionCount = glfwExtensionCount,
    .ppEnabledExtensionNames = glfwExtensions
  };

  if (vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS) {
    throw std::runtime_error("Could not create vulkan instance");
  }

  auto pyhsicalDeviceCount = uint32_t{0};
  vkEnumeratePhysicalDevices(vkInstance, &pyhsicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(pyhsicalDeviceCount);
  vkEnumeratePhysicalDevices(vkInstance, &pyhsicalDeviceCount, physicalDevices.data());
  physicalDevice = physicalDevices[0];

  auto queuePriority = float{1.0f};
  VkDeviceQueueCreateInfo logicalDeviceQueueCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = 0,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority
  };

  VkPhysicalDeviceFeatures logicalDeviceFeatures{};
  VkDeviceCreateInfo logicalDeviceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &logicalDeviceQueueCreateInfo,
    .pEnabledFeatures = &logicalDeviceFeatures
  };

  if (vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
    throw std::runtime_error("Could not create logical device");
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void Viewer::exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}
