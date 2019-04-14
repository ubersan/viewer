#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>
#include <string>

class Viewer {
public:
  ~Viewer();

  void run();
private:
  uint32_t width{640};
  uint32_t height{480};

  GLFWwindow* window;

  VkInstance vkInstance;
  VkPhysicalDevice physicalDevice;
  VkDevice logicalDevice;
  VkQueue graphicsQueue;
  VkSurfaceKHR surface;
  VkQueue presentationQueue;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  VkPipelineLayout pipelineLayout;

  std::vector<const char*> logicalDeviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  void createShaderModuleFromBinary(const std::string& filename, VkShaderModule& shaderModule);

  static void exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
