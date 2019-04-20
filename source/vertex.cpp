#include "vertex.hpp"

Vertex::Vertex(const glm::vec2& position, const glm::vec3 color)
  : position(position), color(color) {}

VkVertexInputBindingDescription Vertex::getBindingDescription() {
  return VkVertexInputBindingDescription{
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
  return {
    VkVertexInputAttributeDescription{
      .location = 0,
      .binding = 0,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(Vertex, position)
    },
    VkVertexInputAttributeDescription{
      .location = 1,
      .binding = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = offsetof(Vertex, color)
    }
  };
}
