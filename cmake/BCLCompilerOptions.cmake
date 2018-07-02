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

# Enable C++ 11 support.
if(BCL_COMPILER_IS_GCC_COMPATIBLE)
  check_cxx_compiler_flag("-std=c++11" CXX_SUPPORTS_CXX11)
  if(CXX_SUPPORTS_CXX11)
    if(CYGWIN OR MINGW)
      # MinGW and Cygwin are a bit stricter and lack things like
      # 'strdup', 'stricmp', etc in c++11 mode.
      bcl_append("-std=gnu++11" CMAKE_CXX_FLAGS)
    else()
      bcl_append("-std=c++11" CMAKE_CXX_FLAGS)
    endif()
  else()
    message(FATAL_ERROR "BCL requires C++11 support but the '-std=c++11' flag isn't supported.")
  endif()
endif(BCL_COMPILER_IS_GCC_COMPATIBLE)

