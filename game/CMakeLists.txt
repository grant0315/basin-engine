add_executable(my_app
  src/main.cpp
)

target_include_directories(my_app
  PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
)

target_link_libraries(my_app
  PRIVATE
    basin_engine
    glad
)

add_custom_target(
  run
  COMMAND $<TARGET_FILE:my_app>
  DEPENDS my_app
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Run my_app (working directory: source root)"
)
