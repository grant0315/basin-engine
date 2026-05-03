add_executable(basin_editor
  src/main.cpp
  src/editor_app.cpp
  src/viewport.cpp
  src/imgui_backends/imgui_impl_glfw.cpp
  src/imgui_backends/imgui_impl_opengl3.cpp
)

target_include_directories(basin_editor
  PRIVATE
    src
    src/imgui_backends
    ${CMAKE_SOURCE_DIR}/engine/include
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
)

target_link_libraries(basin_editor
  PRIVATE
    basin_engine
    glad
    imgui::imgui
    glfw
    OpenGL::GL
    nlohmann_json::nlohmann_json
)

add_custom_target(
  run_editor
  COMMAND $<TARGET_FILE:basin_editor>
  DEPENDS basin_editor
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Run basin_editor (working directory: source root)"
)
