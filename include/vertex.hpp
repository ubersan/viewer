#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <vector>

class Vertex {
public:
  Vertex(const glm::vec2& position, const glm::vec3 color);

  static VkVertexInputBindingDescription getBindingDescription();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

private:
  glm::vec2 position;
  glm::vec3 color;
};