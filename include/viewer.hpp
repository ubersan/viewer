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

  const size_t MAX_FRAMES_IN_FLIGHT{2};

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
  std::vector<VkFramebuffer> swapChainFramebuffers;

  VkRenderPass renderPass;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<const char*> logicalDeviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  size_t currentFrame{0};

  bool resizeHappended{false};

  void drawFrame();
  void createSwapChain();
  void recreateSwapChain();
  void cleanupSwapChain();
  void createShaderModuleFromBinary(const std::string& filename, VkShaderModule& shaderModule);

  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
