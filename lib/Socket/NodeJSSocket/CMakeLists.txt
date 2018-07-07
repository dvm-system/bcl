option(BCL_NODEJS_SOCKET "Enable Node.js socket wrapper library." ON)

unset(BCL_NODEJS_SOCKET_SOURCES CACHE)

if (BCL_NODEJS_SOCKET)
  add_library(NodeJSSocket INTERFACE)
  target_include_directories(NodeJSSocket INTERFACE ${BCL_INCLUDE_DIR})
  target_sources(NodeJSSocket INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/NodeJSSocket.cpp>
    $<INSTALL_INTERFACE:lib/NodeJSSocket.cpp>)
  set(BCL_NODEJS_SOCKET_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/NodeJSSocket.cpp
    CACHE INTERNAL "Sources of NodeJSSocket target in build tree")
  target_link_libraries(NodeJSSocket INTERFACE Core)
  # Do not move this install command under condition because this command
  # add NodeJSSocket target to export file. This file may be used without
  # installation.
  install(TARGETS NodeJSSocket EXPORT BCLExports)
  if(BCL_INSTALL)
    install(FILES NodeJSSocket.cpp DESTINATION lib)
  endif()
endif()
