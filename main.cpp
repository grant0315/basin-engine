#include <glad/glad.h>
// break
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "collision_system.h"
#include "player.h"
#include "primitive_generator.h"
#include "scene.h"
#include "text_renderer.h"

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
  textRenderer.Load("/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf",
                    24);

  // Shader
  Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

  // Scene
  Scene scene;
  if (!scene.loadFromFile("scenes/main_hall.json")) {
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

    // Clear
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();

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
    shader.setUniform("view", view);
    shader.setUniform("projection", projection);
    shader.setUniform("viewPos", player.getPosition());

    // Draw scene
    for (Entity *ent : scene.getEntities()) {
      ent->Draw(shader);
    }

    // HUD
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << "FPS: " << appState.fps;
    textRenderer.RenderText(ss.str(), 10.0f, appState.windowHeight - 20.0f,
                            1.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
