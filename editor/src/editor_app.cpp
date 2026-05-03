#include "editor_app.h"
#include "basin/scene/entity.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

namespace basin {

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

void EditorApp::onInit(Window &window) {
  // ImGui setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window.getNativeWindow(), true);
  ImGui_ImplOpenGL3_Init("#version 330");
  window.setCursorEnabled(true);

  // Viewport
  m_viewport = std::make_unique<Viewport>(800, 600);

  // Shaders
  m_standardShader = std::make_unique<Shader>(
      "shared/shaders/vertex.glsl", "shared/shaders/fragment.glsl");
  m_dotmatrixShader = std::make_unique<Shader>(
      "shared/shaders/vertex.glsl",
      "shared/shaders/dotmatrix_fragment.glsl");
  m_activeShader = m_standardShader.get();

  // Text renderer for HUD (optional in editor)
  m_textRenderer =
      std::make_unique<TextRenderer>(window.getWidth(), window.getHeight());
  const char *fontPath = "shared/fonts/JetBrainsMonoNerdFont-Regular.ttf";
  if (const char *env = std::getenv("BASIN_FONT")) {
    if (env[0] != '\0')
      fontPath = env;
  }
  m_textRenderer->Load(fontPath, 24);

  // Load default scene
  m_scene = std::make_unique<Scene>();
  if (!m_scene->loadFromFile("game/scenes/main_hall.json")) {
    std::cout << "Failed to load default scene" << std::endl;
  }
}

void EditorApp::onUpdate(float deltaTime, Window &window) {
  // ImGui frame start
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  drawMenuBar();
  drawSceneHierarchy();
  drawInspector();
  drawViewport();
}

void EditorApp::onRender() {
  // Render ImGui
  ImGui::Render();
  int displayW, displayH;
  glfwGetFramebufferSize(glfwGetCurrentContext(), &displayW, &displayH);
  glViewport(0, 0, displayW, displayH);
  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorApp::onShutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

// ---------------------------------------------------------------------------
// Panels
// ---------------------------------------------------------------------------

void EditorApp::drawMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
        m_scene->loadFromFile("game/scenes/main_hall.json");
        m_selectedEntity = -1;
      }
      if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
        m_scene->saveToFile("game/scenes/main_hall.json");
        std::cout << "Scene saved!" << std::endl;
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Exit"))
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Use Dot Matrix Preview", nullptr, &m_showDotMatrix);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void EditorApp::drawSceneHierarchy() {
  ImGui::SetNextWindowPos(ImVec2(0, 19), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
  ImGui::Begin("Scene Hierarchy");

  // Entities section
  if (ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Add Cube")) {
      PrimitiveGenerator primGen;
      Entity *ent = new Entity("new_cube", primGen.generateCube(1.0f),
                               glm::vec3(0, 1, 0), glm::quat(),
                               glm::vec3(1, 1, 1), true);
      ent->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
      m_scene->addEntity(ent);
      m_selectedEntity = static_cast<int>(m_scene->getEntities().size()) - 1;
      m_selectionIsLight = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Entity")) {
      if (!m_selectionIsLight && m_selectedEntity >= 0 &&
          m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
        m_scene->removeEntity(static_cast<size_t>(m_selectedEntity));
        m_selectedEntity = -1;
      }
    }

    int idx = 0;
    for (Entity *ent : m_scene->getEntities()) {
      ImGui::PushID(idx);
      bool visible = ent->isVisible();
      if (ImGui::Checkbox("##vis", &visible)) {
        ent->setVisible(visible);
      }
      ImGui::PopID();
      ImGui::SameLine();

      ImGuiTreeNodeFlags flags =
          ((!m_selectionIsLight && m_selectedEntity == idx)
               ? ImGuiTreeNodeFlags_Selected
               : 0) |
          ImGuiTreeNodeFlags_Leaf;
      bool opened = ImGui::TreeNodeEx(ent->getName().c_str(), flags);
      if (ImGui::IsItemClicked()) {
        m_selectedEntity = idx;
        m_selectionIsLight = false;
      }
      if (opened)
        ImGui::TreePop();
      idx++;
    }
  }

  // Lights section
  if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Add Directional")) {
      Light light;
      light.name = "Directional Light " + std::to_string(m_scene->getLights().size() + 1);
      light.type = LightType::Directional;
      light.direction = glm::vec3(0.3f, -1.0f, 0.2f);
      light.color = glm::vec3(1.0f, 1.0f, 1.0f);
      light.intensity = 1.0f;
      m_scene->addLight(light);
      m_selectedLight = static_cast<int>(m_scene->getLights().size()) - 1;
      m_selectionIsLight = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Point")) {
      Light light;
      light.name = "Point Light " + std::to_string(m_scene->getLights().size() + 1);
      light.type = LightType::Point;
      light.position = glm::vec3(0.0f, 5.0f, 0.0f);
      light.color = glm::vec3(1.0f, 1.0f, 1.0f);
      light.intensity = 1.0f;
      m_scene->addLight(light);
      m_selectedLight = static_cast<int>(m_scene->getLights().size()) - 1;
      m_selectionIsLight = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Light")) {
      if (m_selectionIsLight && m_selectedLight >= 0 &&
          m_selectedLight < static_cast<int>(m_scene->getLights().size())) {
        m_scene->removeLight(static_cast<size_t>(m_selectedLight));
        m_selectedLight = -1;
        m_selectionIsLight = false;
      }
    }

    int lidx = 0;
    for (const Light &light : m_scene->getLights()) {
      ImGuiTreeNodeFlags flags =
          ((m_selectionIsLight && m_selectedLight == lidx)
               ? ImGuiTreeNodeFlags_Selected
               : 0) |
          ImGuiTreeNodeFlags_Leaf;
      std::string label = light.name;
      if (light.type == LightType::Directional) label += " [Dir]";
      else if (light.type == LightType::Point) label += " [Pt]";
      bool opened = ImGui::TreeNodeEx(label.c_str(), flags);
      if (ImGui::IsItemClicked()) {
        m_selectedLight = lidx;
        m_selectionIsLight = true;
      }
      if (opened)
        ImGui::TreePop();
      lidx++;
    }
  }

  ImGui::End();
}

void EditorApp::drawInspector() {
  ImGui::SetNextWindowPos(ImVec2(0, 419), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
  ImGui::Begin("Inspector");

  if (m_selectionIsLight && m_selectedLight >= 0 &&
      m_selectedLight < static_cast<int>(m_scene->getLights().size())) {
    Light &light = m_scene->getLights()[m_selectedLight];

    // Name
    char nameBuf[64];
    strncpy(nameBuf, light.name.c_str(), sizeof(nameBuf));
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
      light.name = nameBuf;
    }

    // Type
    const char *types[] = {"Directional", "Point", "Spot"};
    int currentType = static_cast<int>(light.type);
    if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
      light.type = static_cast<LightType>(currentType);
    }

    // Color
    ImGui::ColorEdit3("Color", &light.color.x);

    // Intensity
    ImGui::DragFloat("Intensity", &light.intensity, 0.05f, 0.0f, 100.0f);

    if (light.type == LightType::Directional || light.type == LightType::Spot) {
      ImGui::DragFloat3("Direction", &light.direction.x, 0.05f);
    }

    if (light.type == LightType::Point || light.type == LightType::Spot) {
      ImGui::DragFloat3("Position", &light.position.x, 0.1f);
      ImGui::Text("Attenuation");
      ImGui::DragFloat("Constant", &light.constant, 0.01f, 0.0f, 10.0f);
      ImGui::DragFloat("Linear", &light.linear, 0.001f, 0.0f, 10.0f);
      ImGui::DragFloat("Quadratic", &light.quadratic, 0.0001f, 0.0f, 10.0f);
    }
  } else if (!m_selectionIsLight && m_selectedEntity >= 0 &&
             m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
    Entity *ent = m_scene->getEntities()[m_selectedEntity];

    char nameBuf[64];
    strncpy(nameBuf, ent->getName().c_str(), sizeof(nameBuf));
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
      ent->setName(nameBuf);
    }

    glm::vec3 pos = ent->getPosition();
    if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
      ent->setPosition(pos);
    }

    glm::vec3 euler = glm::degrees(glm::eulerAngles(ent->getRotation()));
    if (ImGui::DragFloat3("Rotation", &euler.x, 1.0f)) {
      ent->setRotationEuler(euler);
    }

    glm::vec3 scale = ent->getScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.05f, 0.01f, 100.0f)) {
      ent->setScale(scale);
    }

    glm::vec4 color = ent->getColor();
    if (ImGui::ColorEdit3("Color", &color.x)) {
      ent->setColor(color);
    }

    bool collidable = ent->isCollidable();
    ImGui::Checkbox("Collidable", &collidable);
  } else {
    ImGui::Text("No selection");
  }

  ImGui::End();
}

void EditorApp::drawViewport() {
  ImGui::SetNextWindowPos(ImVec2(250, 19), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport");

  bool viewportHovered = ImGui::IsWindowHovered();

  // Blender-style camera controls
  if (viewportHovered) {
    ImGuiIO &io = ImGui::GetIO();

    // Zoom with scroll wheel
    float wheel = io.MouseWheel;
    if (wheel != 0.0f) {
      m_orbitDistance = glm::max(1.0f, m_orbitDistance - wheel * 2.0f);
    }

    // Orbit with middle mouse drag
    if (ImGui::IsMouseDragging(2)) {
      ImVec2 delta = ImGui::GetMouseDragDelta(2);
      ImGui::ResetMouseDragDelta(2);

      float orbitSpeed = 0.3f;
      if (io.KeyShift) {
        // Pan: shift + middle mouse
        glm::vec3 right = glm::normalize(
            glm::cross(m_editorCamTarget - m_editorCamPos, glm::vec3(0, 1, 0)));
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::vec3 pan = right * (-delta.x * 0.02f * m_orbitDistance * 0.05f) +
                        up * (delta.y * 0.02f * m_orbitDistance * 0.05f);
        m_editorCamTarget += pan;
      } else {
        // Orbit: middle mouse
        m_orbitYaw -= delta.x * orbitSpeed;
        m_orbitPitch += delta.y * orbitSpeed;
        m_orbitPitch = glm::clamp(m_orbitPitch, -89.0f, 89.0f);
      }
    }

    // Update camera position from spherical coords
    float yawRad = glm::radians(m_orbitYaw);
    float pitchRad = glm::radians(m_orbitPitch);
    glm::vec3 offset;
    offset.x = m_orbitDistance * cos(pitchRad) * cos(yawRad);
    offset.y = m_orbitDistance * sin(pitchRad);
    offset.z = m_orbitDistance * cos(pitchRad) * sin(yawRad);
    m_editorCamPos = m_editorCamTarget + offset;
  }

  ImVec2 avail = ImGui::GetContentRegionAvail();
  if (avail.x > 0 && avail.y > 0) {
    m_viewport->resize(static_cast<int>(avail.x),
                       static_cast<int>(avail.y));
    renderSceneToViewport();

    ImGui::Image(
        reinterpret_cast<ImTextureID>(
            static_cast<uintptr_t>(m_viewport->getColorTexture())),
        avail, ImVec2(0, 1), ImVec2(1, 0));
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void EditorApp::renderSceneToViewport() {
  m_viewport->bind();

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Shader *shader = m_showDotMatrix ? m_dotmatrixShader.get() : m_standardShader.get();
  shader->use();

  // Upload scene lights
  uploadLights(*shader, m_scene->getLights());

  // Editor camera
  glm::mat4 view =
      glm::lookAt(m_editorCamPos, m_editorCamTarget, glm::vec3(0, 1, 0));
  glm::mat4 projection = glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(m_viewport->getWidth()) / m_viewport->getHeight(),
      0.1f, 10000.0f);

  shader->setUniform("view", view);
  shader->setUniform("projection", projection);

  if (m_showDotMatrix) {
    shader->setUniform("dotSize", 4.0f);
    shader->setUniform("maxRadius", 0.32f);
    shader->setUniform("softness", 0.08f);
    shader->setUniform("gridGap", 0.75f);
    shader->setUniform("backgroundColor", glm::vec3(0.02f, 0.02f, 0.02f));
  } else {
    shader->setUniform("viewPos", m_editorCamPos);
  }

  for (Entity *ent : m_scene->getEntities()) {
    ent->Draw(*shader);
  }

  m_viewport->unbind(
      static_cast<int>(ImGui::GetIO().DisplaySize.x),
      static_cast<int>(ImGui::GetIO().DisplaySize.y));
}

} // namespace basin
