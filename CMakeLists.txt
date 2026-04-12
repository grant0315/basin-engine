cmake_minimum_required(VERSION 3.10)

# 1. FIX THE WARNING: Must be set BEFORE find_package(OpenGL)
set(OpenGL_GL_PREFERENCE GLVND)

project(OpenGLProject CXX C)

# 2. FIND PACKAGES
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

# 3. GLAD SETUP
# Ensure the path matches exactly where you extracted the GLAD files
add_library(glad STATIC external/glad/src/glad.c)

# This line tells the compiler to look in external/glad/include 
# so it can find 'glad/glad.h'
target_include_directories(glad PUBLIC external/glad/include)

# 4. APP SETUP
add_executable(my_app main.cpp)

# Link everything together
target_link_libraries(my_app PRIVATE assimp::assimp glad glfw OpenGL::GL)

