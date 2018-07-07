add_executable(sm-example static_map_example.cpp)
target_link_libraries(sm-example Core)

add_executable(stm-example type_map_example.cpp)
target_link_libraries(stm-example Core)

set_target_properties(sm-example stm-example PROPERTIES FOLDER "BCL examples")

if(BCL_INSTALL)
  install(TARGETS sm-example stm-example EXPORT BCLExports DESTINATION bin)
  install(FILES static_map_example.cpp type_map_example.cpp
    DESTINATION example/sm)
endif()
