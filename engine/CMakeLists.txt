add_library(basin_engine STATIC
  src/engine.cpp
  src/renderer/shader.cpp
  src/renderer/mesh.cpp
  src/renderer/model.cpp
  src/renderer/text_renderer.cpp
  src/scene/scene.cpp
  src/scene/primitive_generator.cpp
  src/physics/collision_system.cpp
  src/platform/window.cpp
  src/platform/input_manager.cpp
  src/math/camera.cpp
)

target_include_directories(basin_engine
  PUBLIC
    include
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
  PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/stb
)

target_link_libraries(basin_engine
  PUBLIC
    assimp::assimp
    glad
    glfw
    OpenGL::GL
    Freetype::Freetype
    ${_glm_target}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
  target_link_libraries(basin_engine PUBLIC stdc++fs)
endif()
