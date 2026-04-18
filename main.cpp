#include <glad/glad.h>
// break
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "camera.h"
#include "collision_system.h"
#include "entity.h"
#include "input_manager.h"
#include "player.h"
#include "primitive_generator.h"
#include "text_renderer.h"

const int SCREEN_HEIGHT = 800;
const int SCREEN_WIDTH = 1200;

// Track current window dimensions for dynamic aspect ratio
int currentWidth = SCREEN_WIDTH;
int currentHeight = SCREEN_HEIGHT;

// FPS tracking
double lastTime = 0.0;
int frameCount = 0;
float fps = 0.0f;

// Global text renderer pointer for callbacks
TextRenderer *globalTextRenderer = nullptr;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  // Update current dimensions for projection matrix
  currentWidth = width;
  currentHeight = height;
  // Update text renderer projection
  if (globalTextRenderer) {
    globalTextRenderer->UpdateProjection(width, height);
  }
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window =
      glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello world!", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // Disable VSync for uncapped framerate (or set to 1 for VSync)
  glfwSwapInterval(0);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

  // Set initial viewport
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Init primitive_generator
  PrimitiveGenerator primGen = PrimitiveGenerator();

  // Initialize player
  Player player = Player(primGen.generateCube(10.0f), true);
  player.setPosition(glm::vec3(0.0f, 10.0f, 0.0f));
  player.getInputManager()->SetupCallbacks(window);
  std::cout << "Camera front vector: " << player.getCamera()->GetFront().x
            << ", " << player.getCamera()->GetFront().y << ", "
            << player.getCamera()->GetFront().z << std::endl;

  // Init floor entity
  Entity floorPlane =
      Entity("floor_plane", primGen.generatePlane(1000.0f, 1000.0f), true);
  floorPlane.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

  // Init collision detection system
  CollisionSystem colSys =
      CollisionSystem(std::vector<Entity *>{&player, &floorPlane});

  // Initialize text renderer
  TextRenderer textRenderer(SCREEN_WIDTH, SCREEN_HEIGHT);
  globalTextRenderer = &textRenderer;
  textRenderer.Load("/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf",
                    24);
  std::cout << "Text renderer initialized" << std::endl;

  Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

  Model test_model = Model("assets/low-poly-shop/source/shop_1.fbx");
  Entity test_entity = Entity("shop_front", &test_model, true);
  glm::vec3 scale = glm::vec3(0.03f, 0.03f, 0.03f);
  test_entity.setScale(scale);
  test_entity.setPosition(glm::vec3(0.0f, 0.0f, -20.0f));
  test_entity.setRotationEuler(glm::vec3(0.0f, 0.0f, 0.0f));
  glm::vec3 modelCenter = test_entity.getWorldCenter();

  lastTime = glfwGetTime();
  double lastFrameTime = lastTime;

  while (!glfwWindowShouldClose(window)) {
    // Calculate delta time
    double currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    // Calculate FPS
    frameCount++;
    if (currentTime - lastTime >= 1.0) {
      fps = frameCount / (currentTime - lastTime);
      frameCount = 0;
      lastTime = currentTime;
    }

    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use(); // Activate the shader

    // Handle movement
    player.handleInput(window, deltaTime);

    // Check the users desired position bounding box
    // Determine if that bounding box intersects with another entities current
    // bounding box If so, check what axes do they intersect, and then don't
    // allow movement in that axes
    glm::vec3 playerCurrentPosition = player.getPosition();
    glm::vec3 playerDesiredPosition = player.getDesiredPosition();
    AABB playerCurrentAABB = player.getAxisAlignedBoundingBox();
    AABB playerDesiredAABB =
        player.getAxisAlignedBoundingBoxAtPosition(player.getDesiredPosition());
    AABB floorPlaneAABB = floorPlane.getAxisAlignedBoundingBox();

    glm::vec3 testPosX =
        glm::vec3(playerDesiredPosition.x, playerCurrentPosition.y,
                  playerCurrentPosition.z);
    glm::vec3 testPosY =
        glm::vec3(playerCurrentPosition.x, playerDesiredPosition.y,
                  playerCurrentPosition.z);
    glm::vec3 testPosZ =
        glm::vec3(playerCurrentPosition.x, playerCurrentPosition.y,
                  playerDesiredPosition.z);
    // Get AABB at each test position, and check if it collides with the floor
    // plane
    bool xAxisBlocked, yAxisBlocked, zAxisBlocked;
    xAxisBlocked = colSys.checkAABBCollision(
        player.getAxisAlignedBoundingBoxAtPosition(testPosX), floorPlaneAABB);
    yAxisBlocked = colSys.checkAABBCollision(
        player.getAxisAlignedBoundingBoxAtPosition(testPosY), floorPlaneAABB);
    zAxisBlocked = colSys.checkAABBCollision(
        player.getAxisAlignedBoundingBoxAtPosition(testPosZ), floorPlaneAABB);

    // Determine if moving up or down for proper Y collision handling
    bool movingUp = playerDesiredPosition.y > playerCurrentPosition.y;

    glm::vec3 finalPlayerPosition = glm::vec3(
        (xAxisBlocked ? playerCurrentPosition.x : playerDesiredPosition.x),
        // Allow upward movement even with Y collision (jumping), only block
        // downward
        (yAxisBlocked && !movingUp ? playerCurrentPosition.y
                                   : playerDesiredPosition.y),
        (zAxisBlocked ? playerCurrentPosition.z : playerDesiredPosition.z));

    // Reset vertical velocity if grounded (Y collision)
    if (yAxisBlocked) {
      player.resetVerticalVelocity();
    }

    player.setPosition(finalPlayerPosition);
    player.update(deltaTime);

    // Create and set matrices
    glm::mat4 view = player.getViewMatrix();
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f),
                         (float)currentWidth / currentHeight, 0.1f, 10000.0f);

    shader.setUniform("view", view);
    shader.setUniform("projection", projection);

    shader.setUniform("objectColor", glm::vec3(0.8f, 0.4f, 0.1f));
    test_entity.Draw(shader);
    shader.setUniform("objectColor", glm::vec3(0.3f, 0.3f, 0.3f));
    floorPlane.Draw(shader);

    // Render debug text
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    ss << "FPS: " << fps;
    textRenderer.RenderText(ss.str(), 10.0f, currentHeight - 20.0f, 1.0f,
                            glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
