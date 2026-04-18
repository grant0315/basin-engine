# Basin Engine

A simple 3D rendering engine built with OpenGL.

## Building

```bash
# Create build directory (or use existing)
cmake -B build
cmake --build build

# Run the app
./build/my_app
# Or: cmake --build build --target run
```

Dependencies required:
- GLFW3
- OpenGL
- Assimp
- Freetype

## Features

- OBJ model loading via Assimp
- Basic collision detection (AABB)
- First-person camera controller
- Text rendering with Freetype
- Shader system with vertex/fragment shaders
- Primitive generation (cubes, planes, etc.)
- Scene system with JSON-based scene loading

## Controls

- WASD - Move
- Mouse - Look around
- ESC - Release cursor