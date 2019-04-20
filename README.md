# Simple 3D Viewer

## Setup vulkan

```
export VULKAN_SDK=~/vulkan/1.1.xx.y/x86_64
export PATH=$VULKAN_SDK/bin:$PATH
export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
export VK_LAYER_PATH=$VULKAN_SDK/etc/explicit_layer.d
```

## Compile shaders

```
/vulkan/1.1.101.0/x86_64/bin/glslangValidator -V /source/shaders/basic.[vert|frag]
```

## Build and run

```
mkdir build
cd build
export CXX=/path/to/your/g++
cmake -GNinja -D CMAKE_BUILD_TYPE=Debug -DVulkan_LIBRARY=path/to/so-file -DVulkan_INCLUDE_DIR=path/to/include ../
ninja
./viewer
```
