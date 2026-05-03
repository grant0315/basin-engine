cmake_minimum_required(VERSION 3.16)

if(UNIX AND NOT APPLE)
  set(OpenGL_GL_PREFERENCE GLVND)
endif()

project(OpenGLProject CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /RTC1 /Zi")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

find_package(glfw3 QUIET CONFIG)
if(NOT glfw3_FOUND)
  find_package(glfw3 REQUIRED)
endif()
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)

find_package(glm QUIET CONFIG)
if(glm_FOUND)
  set(_glm_target glm::glm)
else()
  include(FetchContent)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
  )
  FetchContent_MakeAvailable(glm)
  if(TARGET glm::glm)
    set(_glm_target glm::glm)
  else()
    set(_glm_target glm)
  endif()
endif()

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_executable(
  my_app
  main.cpp
  model.cpp
  mesh.cpp
  shader.cpp
  text_renderer.cpp
  camera.cpp
  input_manager.cpp
  collision_system.cpp
  primitive_generator.cpp
  scene.cpp
)
target_include_directories(my_app PRIVATE include external)

target_link_libraries(
  my_app
  PRIVATE
    assimp::assimp
    glad
    glfw
    OpenGL::GL
    Freetype::Freetype
    ${_glm_target}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
  target_link_libraries(my_app PRIVATE stdc++fs)
endif()

add_custom_target(
  run
  COMMAND $<TARGET_FILE:my_app>
  DEPENDS my_app
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Run my_app (working directory: source root)"
)
