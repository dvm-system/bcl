set(BCL_FULL_NAME "Base Construction Library")
set(BCL_NAME "BCL")
set(BCL_VERSION_MAJOR 3)
set(BCL_VERSION_MINOR 0)
set(BCL_VERSION "${BCL_VERSION_MAJOR}.${BCL_VERSION_MINOR}")
set(BCL_STRING "${BCL_NAME} ${BCL_VERSION}")
set(BCL_AUTHOR "Nikita Kataev (kaniandr@gmail.com)")

file(GLOB_RECURSE BCL_HEADERS "*.h")

set(BCL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to BCL" FORCE)
set(ENV{BCL_HEADERS} "${BCL_HEADERS};${CMAKE_CURRENT_BINARY_DIR}/bcl-config.h")

configure_file("bcl-config.h.in" "bcl-config.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
