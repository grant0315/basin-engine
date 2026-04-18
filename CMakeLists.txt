cmake_minimum_required(VERSION 3.10)

# 1. FIX THE WARNING: Must be set BEFORE find_package(OpenGL)
set(OpenGL_GL_PREFERENCE GLVND)

project(OpenGLProject CXX C)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable optimizations for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# 2. FIND PACKAGES
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)

# 3. GLAD SETUP
# Ensure the path matches exactly where you extracted the GLAD files
add_library(glad STATIC external/glad/src/glad.c)

# This line tells the compiler to look in external/glad/include 
# so it can find 'glad/glad.h'
target_include_directories(glad PUBLIC external/glad/include)

# 4. APP SETUP
add_executable(my_app main.cpp model.cpp mesh.cpp shader.cpp text_renderer.cpp camera.cpp input_manager.cpp collision_system.cpp primitive_generator.cpp)

# Include FreeType headers and local include directory
target_include_directories(my_app PRIVATE ${FREETYPE_INCLUDE_DIRS} include external/nlohmann)

# Link everything together
target_link_libraries(my_app PRIVATE assimp::assimp glad glfw OpenGL::GL ${FREETYPE_LIBRARIES})

# 5. CUSTOM RUN TARGET
# Usage: cmake --build build --target run
# Or if using make: make run (from build directory)
add_custom_target(run
    COMMAND my_app
    DEPENDS my_app
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building and running the application..."
)

# Alternative: run from build directory with "make run"
# Or from root: cmake --build build --target run
