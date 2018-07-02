# Base Construction Library (BCL)

This is a portable free C++ library. BCL supports UNIX and Windows operating systems.

CMake can be used to simplify BCL installation. BCL also provides some useful
CMake modules. To use BCL compiler must support C++11.

BCL contains

  - core header files (BCL::Core target if CMake is used);
  - NodeJS based implementation of socket interface (BCL::NodeJSSocket target if CMake is used);
  - CMake modules.

Simple example is presented bellow.

```cmake
cmake_minimum_required(VERSION 3.4.3)

# Set BCL installation pass and find package.
# It is also possible to use add_subdirectory() here to configure BCL
# without installation.
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} <path-to-bcl>)
find_package(BCL 3.0 REQUIRED)

add_executable(Example Example.cpp)

# Set options which is necessary to compile BCL.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${BCL_CMAKE_MODULE_PATH})
include(BCLCompilerOptions)

# Set include directories and enable MSVC to load header files into the project.
target_link_libraries(Example BCL::Core)
```

```C++
//Example.cpp
#include <bcl/bcl-config.h>
#include <iostream>

int main() {
  std::cout << BCL_DESCRIPTION << " (BCL)"
    << " version " << (BCL_VERSION_STRING) << std::endl;
#ifdef BCL_LEGACY
  std::cout << "Installation with legacy code." << std::endl;
#else
  std::cout << "Installation without legacy code." << std::endl;
#endif
#ifdef BCL_NODEJS_SOCKET
  std::cout << "Installation with NodeJSSocket." << std::endl;
#else
  std::cout << "Installation without NodeJSSocket." << std::endl;
#endif
  return 0;
}
```

```bash
Base Construction Library (BCL) version 3.0
Installation without legacy code.
Installation with NodeJSSocket.
```
