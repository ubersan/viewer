#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

class Viewer {
public:
  Viewer();
  ~Viewer();

  void run();
private:
  GLFWwindow* window;
};