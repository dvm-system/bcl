option(BCL_C_SOCKET "Enable C-based socket wrapper library." ON)

if (BCL_C_SOCKET)
  if (MSVC_IDE)
    source_group(bcl FILES ${BCL_CORE_HEADERS})
  endif()

  add_library(BCLCSocket CSocket.cpp)
  target_link_libraries(BCLCSocket Core)
  if (WIN32)
    target_link_libraries(BCLCSocket ws2_32)
  endif()

  set_target_properties(BCLCSocket PROPERTIES FOLDER "BCL libraries")

  if(BCL_INSTALL)
    install(TARGETS BCLCSocket EXPORT BCLExports DESTINATION lib)
  else()
    # Call install() to export library. Use destination inside a build tree.
    install(TARGETS BCLCSocket EXPORT BCLExports
            DESTINATION ${CMAKE_BINARY_DIR}/lib)
  endif()
endif()
