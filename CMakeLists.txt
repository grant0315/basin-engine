cmake_minimum_required(VERSION 3.16)

if(UNIX AND NOT APPLE)
  set(OpenGL_GL_PREFERENCE GLVND)
endif()

project(BasinEngine CXX C)

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

add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)

add_subdirectory(engine)
add_subdirectory(game)
