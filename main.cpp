#include <glad/glad.h>
// break
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "entity.h"

const int SCREEN_HEIGHT = 800;
const int SCREEN_WIDTH = 1200;

float MOUSE_POS_X = (float)SCREEN_WIDTH / 2;
float MOUSE_POS_Y = (float)SCREEN_HEIGHT / 2;

glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 1000.0f);
float cameraSpeed = 10.0f; // adjust for sensitivity

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos) {
  float deltaX = MOUSE_POS_X - xpos;
  float deltaY = MOUSE_POS_Y - ypos;

  // Translate camera angle based on deltaX and deltaY changes
}

void key_movement(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPosition.z -= cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPosition.z += cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPosition.x -= cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPosition.x += cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    cameraPosition.y += cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    cameraPosition.y -= cameraSpeed;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    std::cout << "Closing GLFW window" << std::endl;
    glfwSetWindowShouldClose(window, GLFW_TRUE);
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

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_cursor_callback);
  glfwSetKeyCallback(window, key_callback);

  // Disable face culling
  // glDisable(GL_CULL_FACE);
  //
  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

  Model test_model = Model("assets/couch.obj");
  Entity test_entity = Entity("couch", &test_model);
  glm::vec3 scale = glm::vec3(0.1f, 0.1f, 0.1f);
  test_entity.setScale(scale);
  glm::vec3 modelCenter = test_entity.getWorldCenter();

  glm::mat4 view = glm::lookAt(modelCenter + glm::vec3(0.0f, 0.0f, 1000.0f),
                               modelCenter, glm::vec3(0.0f, 1.0f, 0.0f));
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Check for movement
    key_movement(window);

    shader.use(); // Activate the shader

    // Create and set matrics
    glm::mat4 view =
        glm::lookAt(cameraPosition, modelCenter, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f),
                         (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 10000.0f);

    shader.setUniform("view", view);
    shader.setUniform("projection", projection);

    test_entity.Draw(shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
