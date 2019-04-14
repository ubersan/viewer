# Simple 3D Viewer

Build and run:
```
mkdir build
cd build
export CXX=/path/to/your/g++
cmake -GNinja -D CMAKE_BUILD_TYPE=Debug -DVulkan_LIBRARY=path/to/so-file -DVulkan_INCLUDE_DIR=path/to/include ../
ninja
./viewer
```
