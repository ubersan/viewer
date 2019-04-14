cmake_minimum_required(VERSION 3.10)
set(CMAKE_CSS_STANDARD 17)

project(viewer)

find_package(glfw3 3.2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(
  include
)

add_executable(viewer
  source/viewer.cpp
  source/main.cpp
)

target_link_libraries(viewer glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
