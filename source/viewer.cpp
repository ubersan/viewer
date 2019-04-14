#include "viewer.hpp"
#include <iostream>

Viewer::Viewer() {}

Viewer::~Viewer() {
  glfwTerminate();
  glfwDestroyWindow(window);
}

void Viewer::run() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(640, 480, "3D Viewer", nullptr, nullptr);

  glfwSetKeyCallback(window, exit_key_callback); 

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void Viewer::exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
}