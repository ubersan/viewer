#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

class Viewer {
public:
  Viewer();
  ~Viewer();

  void run();
private:
  GLFWwindow* window;

  static void exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};