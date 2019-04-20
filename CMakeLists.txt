cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

project(viewer)

find_package(glfw3 3.2 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(SYSTEM
  shaderc/libshaderc/include
)

include_directories(
  include
)

add_executable(viewer
  source/vertex.cpp
  source/viewer.cpp
  source/main.cpp
)

target_link_libraries(
  viewer
  glfw ${GLFW_LIBRARIES}
  Vulkan::Vulkan
  ${CMAKE_SOURCE_DIR}/shaderc/build/libshaderc/libshaderc_combined.a
)
