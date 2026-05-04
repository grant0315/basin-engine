#include <glad/glad.h>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "basin/engine.h"
#include "basin/physics/collision_system.h"
#include "basin/player.h"
#include "basin/renderer/shader.h"
#include "basin/renderer/text_renderer.h"
#include "basin/scene/primitive_generator.h"
#include "basin/scene/scene.h"

using namespace basin;

static void uploadLights(Shader &shader, const std::vector<Light> &lights) {
  int count = static_cast<int>(lights.size());
  if (count > 8) count = 8;
  for (int i = 0; i < count; ++i) {
    const Light &l = lights[i];
    std::string base = "uLights[" + std::to_string(i) + "]";
    shader.setUniform(base + ".position", l.position);
    shader.setUniform(base + ".direction", l.direction);
    shader.setUniform(base + ".color", l.color);
    shader.setUniform(base + ".intensity", l.intensity);
    shader.setUniform(base + ".constant", l.constant);
    shader.setUniform(base + ".linear", l.linear);
    shader.setUniform(base + ".quadratic", l.quadratic);
    shader.setUniform(base + ".type", static_cast<int>(l.type));
  }
  shader.setUniform("uLightCount", count);
}

class GameApp : public Application {
public:
  GameApp(const std::string& scenePath) : m_scenePath(scenePath) {}

  void onInit(Window &window) override {
    // Player
    PrimitiveGenerator primGen;
    m_player = std::make_unique<Player>(
        primGen.generateCuboid(1.0f, 1.0f, 3.0f), true,
        glm::vec3(0.0f, 3.0f, 0.0f));
    m_player->getInputManager()->SetupCallbacks(window.getNativeWindow());
    window.setCursorEnabled(false);

    // Text renderer
    m_textRenderer =
        std::make_unique<TextRenderer>(window.getWidth(), window.getHeight());
    const char *fontPath = "shared/fonts/JetBrainsMonoNerdFont-Regular.ttf";
    if (const char *env = std::getenv("BASIN_FONT")) {
      if (env[0] != '\0')
        fontPath = env;
    }
    m_textRenderer->Load(fontPath, 24);
    std::cout << "Text renderer initialized" << std::endl;

    // Shaders
    m_standardShader = std::make_unique<Shader>(
        "shared/shaders/vertex.glsl", "shared/shaders/fragment.glsl");
    m_dotmatrixShader = std::make_unique<Shader>(
        "shared/shaders/vertex.glsl",
        "shared/shaders/dotmatrix_fragment.glsl");
    m_activeShader = m_dotmatrixShader.get();

    // Scene
    m_scene = std::make_unique<Scene>();
    if (!m_scene->loadFromFile(m_scenePath)) {
      std::cout << "Failed to load scene, using fallback" << std::endl;
    }
    m_player->setPosition(m_scene->getSpawnPoint());

    // Collision
    m_colSys = std::make_unique<CollisionSystem>();
    m_colSys->rebuildFromScene(*m_player, m_scene->getEntities());

    // Timing init
    double now = glfwGetTime();
    m_lastFileCheckTime = now;
    m_lastFpsTime = now;
  }

  void onUpdate(float deltaTime, Window &window) override {
    // Update text renderer projection if window resized
    m_textRenderer->UpdateProjection(window.getWidth(), window.getHeight());

    // FPS tracking
    m_frameCount++;
    double now = glfwGetTime();
    if (now - m_lastFpsTime >= 1.0) {
      m_fps = m_frameCount / static_cast<float>(now - m_lastFpsTime);
      m_frameCount = 0;
      m_lastFpsTime = now;
    }

    // Hot-reload scene file (check once per second)
    double currentTime = glfwGetTime();
    if (currentTime - m_lastFileCheckTime >= 1.0) {
      m_lastFileCheckTime = currentTime;
      if (m_scene->hotReloadIfChanged()) {
        m_colSys->rebuildFromScene(*m_player, m_scene->getEntities());
      }
    }

    // Shader toggle (F1)
    bool f1Pressed =
        glfwGetKey(window.getNativeWindow(), GLFW_KEY_F1) == GLFW_PRESS;
    if (f1Pressed && !m_f1PressedLastFrame) {
      m_useDotMatrix = !m_useDotMatrix;
      m_activeShader =
          m_useDotMatrix ? m_dotmatrixShader.get() : m_standardShader.get();
      std::cout << "Switched to "
                << (m_useDotMatrix ? "dot matrix" : "standard") << " shader"
                << std::endl;
    }
    m_f1PressedLastFrame = f1Pressed;

    // Player movement + collision resolution
    m_player->handleInput(window.getNativeWindow(), deltaTime);
    m_colSys->resolveMovement(*m_player, m_scene->getEntities());
    m_player->update(deltaTime);
  }

  void onRender() override {
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_activeShader->use();

    // Camera matrices
    glm::mat4 view = m_player->getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(m_textRenderer->getWidth()) /
            m_textRenderer->getHeight(),
        0.1f, 10000.0f);
    m_activeShader->setUniform("view", view);
    m_activeShader->setUniform("projection", projection);

    // Upload lights
    uploadLights(*m_activeShader, m_scene->getLights());

    if (m_useDotMatrix) {
      m_activeShader->setUniform("dotSize", 4.0f);
      m_activeShader->setUniform("maxRadius", 0.32f);
      m_activeShader->setUniform("softness", 0.08f);
      m_activeShader->setUniform("gridGap", 0.75f);
      m_activeShader->setUniform("backgroundColor",
                                 glm::vec3(0.02f, 0.02f, 0.02f));
    } else {
      m_activeShader->setUniform("viewPos", m_player->getPosition());
    }

    // Draw scene
    for (Entity *ent : m_scene->getEntities()) {
      ent->Draw(*m_activeShader);
    }

    // HUD
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << "FPS: " << m_fps
       << "  [" << (m_useDotMatrix ? "DOT" : "STD") << "] F1=toggle";
    m_textRenderer->RenderText(ss.str(), 10.0f,
                               m_textRenderer->getHeight() - 20.0f, 1.0f,
                               glm::vec3(0.0f, 1.0f, 0.0f));
  }

  void onShutdown() override {
    // unique_ptr handles cleanup
  }

private:
  std::unique_ptr<Player> m_player;
  std::unique_ptr<Scene> m_scene;
  std::unique_ptr<CollisionSystem> m_colSys;
  std::unique_ptr<TextRenderer> m_textRenderer;
  std::unique_ptr<Shader> m_standardShader;
  std::unique_ptr<Shader> m_dotmatrixShader;
  Shader *m_activeShader = nullptr;

  bool m_useDotMatrix = true;
  bool m_f1PressedLastFrame = false;
  double m_lastFileCheckTime = 0.0;

  // FPS tracking
  double m_lastFpsTime = 0.0;
  int m_frameCount = 0;
  float m_fps = 0.0f;
  std::string m_scenePath;
};

int main(int argc, char* argv[]) {
  std::string scenePath = "game/scenes/main_hall.json";
  if (argc > 1) {
    scenePath = argv[1];
  }

  Engine engine(1200, 800, "Basin Engine");
  GameApp game(scenePath);
  engine.run(&game);
  return 0;
}
