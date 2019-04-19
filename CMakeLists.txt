cmake_minimum_required(VERSION 3.10)

set(CMAKE_CSS_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

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
  source/viewer.cpp
  source/main.cpp
)

target_link_libraries(
  viewer
  glfw ${GLFW_LIBRARIES}
  Vulkan::Vulkan
  ${CMAKE_SOURCE_DIR}/shaderc/build/libshaderc/libshaderc_combined.a
)
