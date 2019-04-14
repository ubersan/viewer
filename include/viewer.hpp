#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class Viewer {
public:
  ~Viewer();

  void run();
private:
  GLFWwindow* window;

  VkInstance vkInstance;
  VkPhysicalDevice physicalDevice;
  VkDevice logicalDevice;

  static void exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};