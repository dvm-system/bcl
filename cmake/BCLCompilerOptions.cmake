# Determine type of compiler to specify flags properly.
if(CMAKE_COMPILER_IS_GNUCXX)
  set(BCL_COMPILER_IS_GCC_COMPATIBLE ON)
elseif(MSVC)
  set(BCL_COMPILER_IS_GCC_COMPATIBLE OFF)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(BCL_COMPILER_IS_GCC_COMPATIBLE ON)
endif()

include(CheckCXXCompilerFlag)
include(BCLUtils)
