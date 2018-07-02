add_library(NodeJSSocket INTERFACE)
target_include_directories(NodeJSSocket INTERFACE ${BCL_INCLUDE_DIR})
#  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
#  $<INSTALL_INTERFACE:include>)
target_link_libraries(NodeJSSocket INTERFACE Core)
install(TARGETS NodeJSSocket EXPORT BCLExports)
install(FILES NodeJSSocket.cpp DESTINATION lib)
