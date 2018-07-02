add_library(NodeJSSocket INTERFACE)
target_include_directories(NodeJSSocket INTERFACE ${BCL_INCLUDE_DIR})
target_sources(NodeJSSocket INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/NodeJSSocket.cpp>
  $<INSTALL_INTERFACE:lib/NodeJSSocket.cpp>)
target_link_libraries(NodeJSSocket INTERFACE Core)
install(TARGETS NodeJSSocket EXPORT BCLExports)
install(FILES NodeJSSocket.cpp DESTINATION lib)
