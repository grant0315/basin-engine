#include <glad/glad.h>
// break
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "basin/physics/collision_system.h"
#include "basin/player.h"
#include "basin/scene/primitive_generator.h"
#include "basin/scene/scene.h"
#include "basin/renderer/text_renderer.h"

// ---------------------------------------------------------------------------
// Application state — passed to GLFW callbacks via glfwSetWindowUserPointer
// ---------------------------------------------------------------------------
struct AppState {
  int windowWidth;
  int windowHeight;
  TextRenderer *textRenderer = nullptr;

  // Timing
  double lastFpsTime = 0.0;
  double lastFrameTime = 0.0;
  double lastFileCheckTime = 0.0;
  int frameCount = 0;
  float fps = 0.0f;

  void updateTiming(double currentTime, float &outDeltaTime) {
    outDeltaTime = static_cast<float>(currentTime - lastFrameTime);
    lastFrameTime = currentTime;

    frameCount++;
    if (currentTime - lastFpsTime >= 1.0) {
      fps = frameCount / static_cast<float>(currentTime - lastFpsTime);
      frameCount = 0;
      lastFpsTime = currentTime;
    }
  }
};

// ---------------------------------------------------------------------------
// GLFW callbacks
// ---------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  AppState *state =
      static_cast<AppState *>(glfwGetWindowUserPointer(window));
  if (!state)
    return;
  state->windowWidth = width;
  state->windowHeight = height;
  if (state->textRenderer) {
    state->textRenderer->UpdateProjection(width, height);
  }
}

// ---------------------------------------------------------------------------
// Window / GL bootstrap
// ---------------------------------------------------------------------------
GLFWwindow *initWindow(int width, int height, const char *title) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return nullptr;
  }

  glfwSwapInterval(0); // Uncapped framerate (set to 1 for VSync)
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

  glViewport(0, 0, width, height);
  glEnable(GL_DEPTH_TEST);

  return window;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main() {
  const int SCREEN_WIDTH = 1200;
  const int SCREEN_HEIGHT = 800;

  GLFWwindow *window = initWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello world!");
  if (!window)
    return -1;

  // Application state (replaces globals)
  AppState appState;
  appState.windowWidth = SCREEN_WIDTH;
  appState.windowHeight = SCREEN_HEIGHT;
  glfwSetWindowUserPointer(window, &appState);

  // Player
  PrimitiveGenerator primGen;
  Player player(primGen.generateCuboid(1.0f, 1.0f, 3.0f), true,
                glm::vec3(0.0f, 3.0f, 0.0f));
  player.getInputManager()->SetupCallbacks(window);

  // Text renderer
  TextRenderer textRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
  appState.textRenderer = &textRenderer;
  const char *fontPath = "shared/fonts/JetBrainsMonoNerdFont-Regular.ttf";
  if (const char *env = std::getenv("BASIN_FONT")) {
    if (env[0] != '\0') {
      fontPath = env;
    }
  }
  textRenderer.Load(fontPath, 24);
  std::cout << "Text renderer initialized" << std::endl;

  // Shaders
  Shader standardShader("shared/shaders/vertex.glsl",
                        "shared/shaders/fragment.glsl");
  Shader dotmatrixShader("shared/shaders/vertex.glsl",
                         "shared/shaders/dotmatrix_fragment.glsl");
  Shader *activeShader = &dotmatrixShader;
  bool useDotMatrix = true;
  bool f1PressedLastFrame = false;

  // Scene
  Scene scene;
  if (!scene.loadFromFile("game/scenes/main_hall.json")) {
    std::cout << "Failed to load scene, using fallback" << std::endl;
  }
  player.setPosition(scene.getSpawnPoint());

  // Collision
  CollisionSystem colSys;
  colSys.rebuildFromScene(player, scene.getEntities());

  // Timing init
  double now = glfwGetTime();
  appState.lastFpsTime = now;
  appState.lastFrameTime = now;
  appState.lastFileCheckTime = now;

  // ---- Game loop ----------------------------------------------------------
  while (!glfwWindowShouldClose(window)) {
    double currentTime = glfwGetTime();
    float deltaTime;
    appState.updateTiming(currentTime, deltaTime);

    // Hot-reload scene file (check once per second)
    if (currentTime - appState.lastFileCheckTime >= 1.0) {
      appState.lastFileCheckTime = currentTime;
      if (scene.hotReloadIfChanged()) {
        colSys.rebuildFromScene(player, scene.getEntities());
      }
    }

    // Shader toggle (F1)
    bool f1Pressed = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
    if (f1Pressed && !f1PressedLastFrame) {
      useDotMatrix = !useDotMatrix;
      activeShader = useDotMatrix ? &dotmatrixShader : &standardShader;
      std::cout << "Switched to "
                << (useDotMatrix ? "dot matrix" : "standard") << " shader"
                << std::endl;
    }
    f1PressedLastFrame = f1Pressed;

    // Clear
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    activeShader->use();

    // Player movement + collision resolution
    player.handleInput(window, deltaTime);
    colSys.resolveMovement(player, scene.getEntities());
    player.update(deltaTime);

    // Camera matrices
    glm::mat4 view = player.getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(appState.windowWidth) / appState.windowHeight, 0.1f,
        10000.0f);
    activeShader->setUniform("view", view);
    activeShader->setUniform("projection", projection);

    if (useDotMatrix) {
      activeShader->setUniform("lightPos", glm::vec3(15.0f, 18.0f, 15.0f));
      activeShader->setUniform("dotSize", 4.0f);
      activeShader->setUniform("maxRadius", 0.32f);
      activeShader->setUniform("softness", 0.08f);
      activeShader->setUniform("gridGap", 0.75f);
      activeShader->setUniform("backgroundColor",
                               glm::vec3(0.02f, 0.02f, 0.02f));
    } else {
      activeShader->setUniform("viewPos", player.getPosition());
    }

    // Draw scene
    for (Entity *ent : scene.getEntities()) {
      ent->Draw(*activeShader);
    }

    // HUD
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << "FPS: " << appState.fps
       << "  [" << (useDotMatrix ? "DOT" : "STD") << "] F1=toggle";
    textRenderer.RenderText(ss.str(), 10.0f, appState.windowHeight - 20.0f,
                            1.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
