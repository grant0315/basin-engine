#include "editor_app.h"
#include "basin/scene/entity.h"
#include "basin/scene/collection.h"
#include "basin/renderer/model.h"
#include "basin/renderer/material.h"
#include "basin/renderer/texture_loader.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <portable-file-dialogs.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

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

static bool copyFileToDir(const std::string& srcPath, const std::string& destDir) {
  namespace fs = std::filesystem;
  fs::path src(srcPath);
  if (!fs::exists(src)) return false;
  fs::path dest = fs::path(destDir) / src.filename();
  fs::create_directories(fs::path(destDir));
  std::error_code ec;
  fs::copy_file(src, dest, fs::copy_options::overwrite_existing, ec);
  return !ec;
}

static std::vector<std::string> getTextureExtensions() {
  return {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".tif", ".tiff", ".gif", ".webp"};
}

static bool isTextureFile(const std::string& ext) {
  std::string lower = ext;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  for (const auto& texExt : getTextureExtensions()) {
    if (lower == texExt) return true;
  }
  return false;
}

static std::vector<std::string> copyTexturesFromDir(const std:: string& modelDir, const std::string& texDestDir) {
  namespace fs = std::filesystem;
  std::vector<std::string> copied;
  fs::path dir(modelDir);
  if (!fs::exists(dir)) return copied;

  for (const auto& entry : fs::recursive_directory_iterator(dir)) {
    if (!entry.is_regular_file()) continue;
    std::string ext = entry.path().extension().string();
    if (isTextureFile(ext)) {
      fs::path srcFile = entry.path();
      fs::path relPath = fs::relative(srcFile, dir);
      fs::path destFile = fs::path(texDestDir) / relPath;
      fs::create_directories(destFile.parent_path());
      std::error_code ec;
      fs::copy_file(srcFile, destFile, fs::copy_options::overwrite_existing, ec);
      if (!ec) {
        copied.push_back(destFile.string());
      }
    }
  }
  return copied;
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
  m_pbrShader = std::make_unique<Shader>(
      "shared/shaders/vertex.glsl", "shared/shaders/fragment_pbr.glsl");
  m_unlitShader = std::make_unique<Shader>(
      "shared/shaders/vertex.glsl", "shared/shaders/fragment_unlit.glsl");
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

  createGrid();

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

  // New Scene modal popup
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("New Scene##modal", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Enter scene name:");
    static char nameBuf[64] = "Untitled";
    ImGui::InputText("##name", nameBuf, sizeof(nameBuf));
    if (ImGui::Button("Create", ImVec2(120, 0))) {
      m_scene->resetToEmpty(nameBuf);
      m_scene->setSpawnPoint(glm::vec3(0.0f, 5.0f, 10.0f));
      m_selectedEntity = -1;
      m_selectedLight = -1;
      m_selectionIsLight = false;
      m_selectedCollection = nullptr;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  // Delete collection confirmation popup
  if (m_confirmDeleteCollection) {
    ImGui::OpenPopup("Delete Collection?##modal");
    m_confirmDeleteCollection = false;
  }
  if (ImGui::BeginPopupModal("Delete Collection?##modal", nullptr,
                              ImGuiWindowFlags_AlwaysAutoResize)) {
    std::string colName = m_selectedCollection ? m_selectedCollection->getName() : "";
    int entCount = m_selectedCollection ? static_cast<int>(m_selectedCollection->collectAllEntities().size()) : 0;
    ImGui::Text("Delete collection \"%s\"?", colName.c_str());
    ImGui::Text("Entities in this collection (%d) will become ungrouped.", entCount);
    if (ImGui::Button("Delete", ImVec2(120, 0))) {
      if (m_selectedCollection) {
        m_scene->removeCollection(m_selectedCollection);
        m_selectedCollection = nullptr;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  drawSceneHierarchy();
  drawInspector();
  drawViewport();

  // Delete key removes current selection
  if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
    if (!m_selectionIsLight && !m_selectedCollection && m_selectedEntity >= 0 &&
        m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
      m_scene->removeEntity(static_cast<size_t>(m_selectedEntity));
      m_selectedEntity = -1;
    } else if (m_selectionIsLight && m_selectedLight >= 0 &&
               m_selectedLight < static_cast<int>(m_scene->getLights().size())) {
      m_scene->removeLight(static_cast<size_t>(m_selectedLight));
      m_selectedLight = -1;
      m_selectionIsLight = false;
    } else if (m_selectedCollection) {
      m_confirmDeleteCollection = true;
    }
  }

  // Home key resets camera
  if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
    m_editorCamPos = glm::vec3(0.0f, 10.0f, 20.0f);
    m_editorCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    m_orbitDistance = 20.0f;
    m_orbitYaw = -90.0f;
    m_orbitPitch = 30.0f;
  }
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

  if (m_gridVAO) glDeleteVertexArrays(1, &m_gridVAO);
  if (m_gridVBO) glDeleteBuffers(1, &m_gridVBO);
}

void EditorApp::createGrid() {
  const int gridHalf = 50;
  const float y = 0.0f;
  std::vector<float> vertices;

  for (int i = -gridHalf; i <= gridHalf; i++) {
    float pos = static_cast<float>(i);

    vertices.push_back(pos); vertices.push_back(y); vertices.push_back(static_cast<float>(-gridHalf));
    vertices.push_back(pos); vertices.push_back(y); vertices.push_back(static_cast<float>(gridHalf));

    vertices.push_back(static_cast<float>(-gridHalf)); vertices.push_back(y); vertices.push_back(pos);
    vertices.push_back(static_cast<float>(gridHalf)); vertices.push_back(y); vertices.push_back(pos);
  }

  m_gridVertexCount = static_cast<int>(vertices.size()) / 3;

  glGenVertexArrays(1, &m_gridVAO);
  glGenBuffers(1, &m_gridVBO);
  glBindVertexArray(m_gridVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void EditorApp::drawGrid() {
  if (m_gridVertexCount == 0) return;

  Shader* shader = m_showDotMatrix ? m_dotmatrixShader.get() : m_standardShader.get();

  glm::mat4 gridModel = glm::mat4(1.0f);
  shader->setUniform("model", gridModel);
  glm::mat3 normalMat = glm::mat3(1.0f);
  shader->setUniform("normalMatrix", normalMat);
  shader->setUniform("objectColor", glm::vec3(0.35f, 0.35f, 0.35f));
  shader->setUniform("objectAlpha", 0.4f);
  shader->setUniform("hasTexture", false);
  shader->setUniform("hasNormalMap", false);
  shader->setUniform("hasHeightMap", false);
  shader->setUniform("hasRoughnessMap", false);

  glBindVertexArray(m_gridVAO);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDrawArrays(GL_LINES, 0, m_gridVertexCount);
  glDisable(GL_BLEND);
  glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// Panels
// ---------------------------------------------------------------------------

void EditorApp::drawMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene...", "Ctrl+N")) {
        ImGui::OpenPopup("New Scene##modal");
      }
      if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
        auto selection = pfd::open_file(
            "Open Scene",
            "game/scenes",
            {"JSON Files", "*.json"},
            pfd::opt::none).result();
        if (!selection.empty()) {
          m_scene->loadFromFile(selection[0]);
          m_selectedEntity = -1;
          m_selectedLight = -1;
          m_selectionIsLight = false;
          m_selectedCollection = nullptr;
        }
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
        std::string path = m_scene->getFilepath();
        if (path.empty()) {
          auto destination = pfd::save_file(
              "Save Scene",
              "game/scenes/" + m_scene->getName() + ".json",
              {"JSON Files", "*.json"}).result();
          if (!destination.empty()) {
            path = destination;
          }
        }
        if (!path.empty()) {
          m_scene->saveToFile(path);
          std::cout << "Scene saved: " << path << std::endl;
        }
      }
      if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
        auto destination = pfd::save_file(
            "Save Scene As",
            "game/scenes/" + m_scene->getName() + ".json",
            {"JSON Files", "*.json"}).result();
        if (!destination.empty()) {
          m_scene->saveToFile(destination);
          std::cout << "Scene saved: " << destination << std::endl;
        }
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
    if (ImGui::BeginMenu("Play")) {
      if (ImGui::MenuItem("Play in Game", "F5")) {
        std::string scenePath = m_scene->getFilepath();
        if (scenePath.empty()) {
          scenePath = "game/scenes/_editor_play_scene.json";
          m_scene->saveToFile(scenePath);
        } else {
          m_scene->saveToFile(scenePath);
        }

        std::string gameExe = "build/game/my_app";
#ifdef _WIN32
        gameExe = "build\\game\\my_app.exe";
#endif

        std::string command = gameExe + " \"" + scenePath + "\"";

#ifdef _WIN32
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        std::wstring wCommand(command.begin(), command.end());

        if (CreateProcessW(nullptr, wCommand.data(), nullptr, nullptr, FALSE,
                            0, nullptr, nullptr, &si, &pi)) {
          WaitForSingleObject(pi.hProcess, INFINITE);
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);
        } else {
          std::cerr << "ERROR: Failed to launch game process." << std::endl;
        }
#else
        int ret = system(command.c_str());
        (void)ret;
#endif
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  // Scene stats overlay at bottom of screen
  {
    float statsHeight = 20.0f;
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, statsHeight));
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - statsHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGuiWindowFlags statsFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("##stats", nullptr, statsFlags)) {
      int entCount = static_cast<int>(m_scene->getEntities().size());
      int lightCount = static_cast<int>(m_scene->getLights().size());
      int colCount = 0;
      std::vector<Collection*> allCols = m_scene->getAllCollections();
      colCount = static_cast<int>(allCols.size());
      ImGui::Text("Entities: %d  |  Lights: %d  |  Collections: %d  |  FPS: %.0f",
                  entCount, lightCount, colCount, ImGui::GetIO().Framerate);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
  }
}

void EditorApp::drawSceneHierarchy() {
  ImGui::SetNextWindowPos(ImVec2(0, 19), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
  ImGui::Begin("Scene Hierarchy");

  // Collections section
  if (ImGui::CollapsingHeader("Collections", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::Button("Add Collection")) {
      std::string name = "Collection " + std::to_string(m_scene->getRootCollections().size() + 1);
      m_scene->addCollection(name);
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Collection") && m_selectedCollection) {
      m_confirmDeleteCollection = true;
    }

    int nodeIndex = 0;
    drawCollectionTree(m_scene->getRootCollections(), nodeIndex);
  }

  // Ungrouped entities section
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
      m_selectedCollection = nullptr;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Plane")) {
      PrimitiveGenerator primGen;
      Entity *ent = new Entity("new_plane", primGen.generatePlane(10.0f, 10.0f, 1.0f),
                               glm::vec3(0, 0, 0), glm::quat(),
                               glm::vec3(1, 1, 1), true);
      ent->setColor(glm::vec4(0.5f, 0.5f, 0.6f, 1.0f));
      ent->setPrimitiveParams(PrimitiveParams{"plane", 10.0f, 10.0f, 1.0f, 0, 0, 0});
      m_scene->addEntity(ent);
      m_selectedEntity = static_cast<int>(m_scene->getEntities().size()) - 1;
      m_selectionIsLight = false;
      m_selectedCollection = nullptr;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Cuboid")) {
      PrimitiveGenerator primGen;
      Entity *ent = new Entity("new_cuboid", primGen.generateCuboid(2.0f, 1.0f, 3.0f),
                               glm::vec3(0, 0.5f, 0), glm::quat(),
                               glm::vec3(1, 1, 1), true);
      ent->setColor(glm::vec4(0.6f, 0.4f, 0.3f, 1.0f));
      ent->setPrimitiveParams(PrimitiveParams{"cuboid", 0, 0, 0, 2.0f, 1.0f, 3.0f});
      m_scene->addEntity(ent);
      m_selectedEntity = static_cast<int>(m_scene->getEntities().size()) - 1;
      m_selectionIsLight = false;
      m_selectedCollection = nullptr;
    }
    ImGui::SameLine();
    if (ImGui::Button("Model...")) {
      auto selection = pfd::open_file(
          "Select Model File", "",
          std::vector<std::string>{"3D Models", "*.glb *.gltf *.fbx *.obj *.dae *.3ds", "All Files", "*.*"},
          pfd::opt::none).result();
      if (!selection.empty()) {
        namespace fs = std::filesystem;
        std::string srcPath = selection[0];
        fs::path srcFile(srcPath);

        std::string modelsDir = "shared/assets/models";
        std::string texturesDir = "shared/assets/textures";
        fs::create_directories(fs::path(modelsDir));
        fs::create_directories(fs::path(texturesDir));

        std::string destModelPath = (fs::path(modelsDir) / srcFile.filename()).string();
        if (copyFileToDir(srcPath, modelsDir)) {
          std::string modelDir = srcFile.parent_path().string();
          copyTexturesFromDir(modelDir + "/textures", texturesDir);
          copyTexturesFromDir(modelDir + "/Textures", texturesDir);
          copyTexturesFromDir(modelDir, texturesDir);

          std::string modelName = srcFile.stem().string();
          Model* model = new Model(destModelPath, texturesDir);
          Entity* ent = new Entity(modelName, model, glm::vec3(0, 0, 0), glm::quat(),
                                   glm::vec3(1, 1, 1), false);
          ent->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

          ModelParams mp;
          mp.modelPath = destModelPath;
          mp.texturesFolder = texturesDir;
          ent->setModelParams(mp);

          m_scene->addEntity(ent);
          m_selectedEntity = static_cast<int>(m_scene->getEntities().size()) - 1;
          m_selectionIsLight = false;
          m_selectedCollection = nullptr;
        } else {
          std::cerr << "ERROR: Failed to copy model file to assets directory." << std::endl;
        }
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Entity")) {
      if (!m_selectionIsLight && !m_selectedCollection && m_selectedEntity >= 0 &&
          m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
        m_scene->removeEntity(static_cast<size_t>(m_selectedEntity));
        m_selectedEntity = -1;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Duplicate")) {
      if (!m_selectionIsLight && !m_selectedCollection && m_selectedEntity >= 0 &&
          m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
        Entity* src = m_scene->getEntities()[m_selectedEntity];
        Model* model = nullptr;
        if (src->hasPrimitiveParams()) {
          PrimitiveGenerator primGen;
          const auto& p = src->getPrimitiveParams();
          if (p.primitiveType == "cube") model = primGen.generateCube(p.size);
          else if (p.primitiveType == "plane") model = primGen.generatePlane(p.width, p.depth, p.thickness);
          else if (p.primitiveType == "cuboid") model = primGen.generateCuboid(p.length, p.width, p.height);
        } else if (src->hasModelParams()) {
          const auto& mp = src->getModelParams();
          ModelTextures mt;
          mt.baseColor = mp.baseColor;
          mt.normal = mp.normal;
          mt.height = mp.height;
          mt.roughness = mp.roughness;
          model = new Model(mp.modelPath, mp.texturesFolder, mt);
        }
        if (model) {
          Entity* dup = new Entity(src->getName() + " (copy)", model,
                                   src->getPosition() + glm::vec3(1, 0, 1),
                                   src->getRotation(), src->getScale(),
                                   src->isCollidable());
          dup->setColor(src->getColor());
          dup->setVisible(src->isVisible());
          if (src->hasPrimitiveParams()) dup->setPrimitiveParams(src->getPrimitiveParams());
          if (src->hasModelParams()) dup->setModelParams(src->getModelParams());
          if (src->getCollection()) src->getCollection()->addEntity(dup);
          m_scene->addEntity(dup);
          m_selectedEntity = static_cast<int>(m_scene->getEntities().size()) - 1;
        }
      }
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
      if (!m_selectionIsLight && !m_selectedCollection && m_selectedEntity > 0 &&
          m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
        m_scene->swapEntities(static_cast<size_t>(m_selectedEntity),
                              static_cast<size_t>(m_selectedEntity - 1));
        m_selectedEntity--;
      }
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
      if (!m_selectionIsLight && !m_selectedCollection && m_selectedEntity >= 0 &&
          m_selectedEntity < static_cast<int>(m_scene->getEntities().size()) - 1) {
        m_scene->swapEntities(static_cast<size_t>(m_selectedEntity),
                              static_cast<size_t>(m_selectedEntity + 1));
        m_selectedEntity++;
      }
    }

    int idx = 0;
    for (Entity *ent : m_scene->getEntities()) {
      // Only show entities not in any collection here
      if (ent->getCollection()) {
        idx++;
        continue;
      }

      bool effectVisible = ent->isVisible();
      bool grayed = !effectVisible;

      ImGui::PushID(idx);
      bool visible = ent->isVisible();
      if (ImGui::Checkbox("##vis", &visible)) {
        ent->setVisible(visible);
      }
      ImGui::PopID();
      ImGui::SameLine();

      if (grayed) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
      }

      ImGuiTreeNodeFlags flags =
          ((!m_selectionIsLight && !m_selectedCollection && m_selectedEntity == idx)
               ? ImGuiTreeNodeFlags_Selected
               : 0) |
          ImGuiTreeNodeFlags_Leaf;
      bool opened = ImGui::TreeNodeEx(ent->getName().c_str(), flags);
      if (ImGui::IsItemClicked()) {
        m_selectedEntity = idx;
        m_selectionIsLight = false;
        m_selectedCollection = nullptr;
      }
      if (opened)
        ImGui::TreePop();

      if (grayed) {
        ImGui::PopStyleColor();
      }
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
      m_selectedCollection = nullptr;
      m_selectedEntity = -1;
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
      m_selectedCollection = nullptr;
      m_selectedEntity = -1;
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
        m_selectedCollection = nullptr;
        m_selectedEntity = -1;
      }
      if (opened)
        ImGui::TreePop();
      lidx++;
    }
  }

  ImGui::End();
}

void EditorApp::drawCollectionTree(const std::vector<Collection*>& collections, int& nodeIndex) {
  for (Collection* col : collections) {
    ImGui::PushID(nodeIndex);

    // Visibility eye icon
    bool visible = col->isVisible();
    if (ImGui::Checkbox("##cvis", &visible)) {
      col->setVisible(visible);
    }
    ImGui::SameLine();

    // Collection tree node
    bool isSelected = (m_selectedCollection == col);
    ImGuiTreeNodeFlags flags = isSelected ? ImGuiTreeNodeFlags_Selected : 0;
    if (col->getChildren().empty() && col->getEntities().empty()) {
      flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool effectVisible = col->isEffectivelyVisible();
    if (!effectVisible) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    }

    std::string label = col->getName() + " [" + std::to_string(col->getEntities().size()) + "]";
    bool opened = ImGui::TreeNodeEx(label.c_str(), flags);
    if (ImGui::IsItemClicked()) {
      m_selectedCollection = col;
      m_selectedEntity = -1;
      m_selectionIsLight = false;
      m_collectionDeltaPos = glm::vec3(0.0f);
    }

    if (!effectVisible) {
      ImGui::PopStyleColor();
    }

    if (opened) {
      // Render child collections
      int childIdx = 0;
      drawCollectionTree(col->getChildren(), childIdx);

      // Render entities in this collection
      int entIdx = 0;
      for (Entity* ent : col->getEntities()) {
        ImGui::PushID(1000 + entIdx);
        bool entVisible = ent->isVisible();
        if (ImGui::Checkbox("##evis", &entVisible)) {
          ent->setVisible(entVisible);
        }
        ImGui::PopID();
        ImGui::SameLine();

        bool entGrayed = !ent->isVisible() || !effectVisible;
        if (entGrayed) {
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        ImGuiTreeNodeFlags entFlags = ImGuiTreeNodeFlags_Leaf;
        // Find global entity index for selection
        int globalIdx = 0;
        for (int i = 0; i < static_cast<int>(m_scene->getEntities().size()); i++) {
          if (m_scene->getEntities()[i] == ent) {
            globalIdx = i;
            break;
          }
        }
        if (!m_selectionIsLight && !m_selectedCollection && m_selectedEntity == globalIdx) {
          // Don't highlight here since collection is selected
        }

        bool entOpened = ImGui::TreeNodeEx(ent->getName().c_str(), ImGuiTreeNodeFlags_Leaf);
        if (ImGui::IsItemClicked()) {
          m_selectedEntity = globalIdx;
          m_selectionIsLight = false;
          m_selectedCollection = nullptr;
        }
        if (entOpened)
          ImGui::TreePop();

        if (entGrayed) {
          ImGui::PopStyleColor();
        }
        entIdx++;
      }

      ImGui::TreePop();
    }

    ImGui::PopID();
    nodeIndex++;
  }
}

void EditorApp::drawInspector() {
  ImGui::SetNextWindowPos(ImVec2(0, 419), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
  ImGui::Begin("Inspector");

  if (m_selectionIsLight && m_selectedLight >= 0 &&
      m_selectedLight < static_cast<int>(m_scene->getLights().size())) {
    Light &light = m_scene->getLights()[m_selectedLight];

    char nameBuf[64];
    strncpy(nameBuf, light.name.c_str(), sizeof(nameBuf));
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
      light.name = nameBuf;
    }

    const char *types[] = {"Directional", "Point", "Spot"};
    int currentType = static_cast<int>(light.type);
    if (ImGui::Combo("Type", &currentType, types, IM_ARRAYSIZE(types))) {
      light.type = static_cast<LightType>(currentType);
    }

    ImGui::ColorEdit3("Color", &light.color.x);
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
  } else if (m_selectedCollection) {
    Collection* col = m_selectedCollection;

    char nameBuf[64];
    strncpy(nameBuf, col->getName().c_str(), sizeof(nameBuf));
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
      col->setName(nameBuf);
    }

    bool visible = col->isVisible();
    if (ImGui::Checkbox("Visible", &visible)) {
      col->setVisible(visible);
    }

    int entCount = static_cast<int>(col->getEntities().size());
    int childCount = static_cast<int>(col->getChildren().size());
    std::vector<Entity*> allEnts = col->collectAllEntities();
    int totalCount = static_cast<int>(allEnts.size());
    ImGui::Text("Direct entities: %d", entCount);
    ImGui::Text("Child collections: %d", childCount);
    ImGui::Text("Total entities: %d", totalCount);

    ImGui::Separator();
    ImGui::Text("Delta Transform (applies to all entities)");

    if (ImGui::DragFloat3("Position Offset", &m_collectionDeltaPos.x, 0.1f)) {
      glm::vec3 delta = m_collectionDeltaPos;
      for (Entity* ent : allEnts) {
        ent->setPosition(ent->getPosition() + delta);
      }
      m_collectionDeltaPos = glm::vec3(0.0f);
    }

    ImGui::Separator();
    ImGui::Text("Move selected entity into this collection:");

    if (!m_selectionIsLight && m_selectedEntity >= 0 &&
        m_selectedEntity < static_cast<int>(m_scene->getEntities().size())) {
      Entity* ent = m_scene->getEntities()[m_selectedEntity];
      if (ent->getCollection() != col) {
        if (ImGui::Button("Move Selected Entity Here")) {
          col->addEntity(ent);
        }
      } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Entity already in this collection");
      }
    }

    // Add child collection
    if (ImGui::Button("Add Child Collection")) {
      std::string childName = col->getName() + " " + std::to_string(col->getChildren().size() + 1);
      m_scene->addCollection(childName, col);
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

    if (ent->hasPrimitiveParams()) {
      ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Type: Primitive (%s)",
                         ent->getPrimitiveParams().primitiveType.c_str());
    } else if (ent->hasModelParams()) {
      ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "Type: Model");
      ImGui::TextWrapped("Path: %s", ent->getModelParams().modelPath.c_str());
    } else {
      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Type: Unknown");
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Focus")) {
      m_editorCamTarget = ent->getWorldCenter();
      m_orbitDistance = 10.0f;
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

    // Material Override section
    ImGui::Separator();
    bool hasOverride = ent->hasMaterialOverride();
    if (ImGui::Checkbox("Material Override", &hasOverride)) {
      if (hasOverride) {
        ent->getMaterialOverride(); // creates default override
      } else {
        ent->clearMaterialOverride();
      }
    }

    if (hasOverride) {
      Material& mat = ent->getMaterialOverride();

      const char* shaderItems[] = {"Standard", "PBR", "DotMatrix", "Unlit"};
      int currentShader = static_cast<int>(mat.shaderType);
      if (ImGui::Combo("Shader", &currentShader, shaderItems, IM_ARRAYSIZE(shaderItems))) {
        mat.shaderType = static_cast<ShaderType>(currentShader);
      }

      ImGui::ColorEdit3("Base Color", &mat.baseColor.x);
      ImGui::DragFloat("Opacity", &mat.opacity, 0.01f, 0.0f, 1.0f);
      ImGui::DragFloat("Roughness", &mat.roughness, 0.01f, 0.0f, 1.0f);
      ImGui::DragFloat("Metallic", &mat.metallic, 0.01f, 0.0f, 1.0f);
      ImGui::DragFloat3("Emissive", &mat.emissive.x, 0.01f, 0.0f, 10.0f);
      ImGui::DragFloat("AO Strength", &mat.aoStrength, 0.01f, 0.0f, 2.0f);

      if (ImGui::TreeNode("Textures")) {
        const char* slotLabels[] = {
          "Diffuse", "Normal", "Height", "Roughness",
          "Metallic", "AO", "Emissive", "Opacity"
        };
        for (int i = 0; i < static_cast<int>(TextureSlot::Count); i++) {
          ImGui::PushID(i);
          bool hasTex = mat.hasTexture[i];
          if (ImGui::Checkbox(slotLabels[i], &hasTex)) {
            if (hasTex) {
              auto result = pfd::open_file(
                  std::string("Select ") + slotLabels[i] + " Texture",
                  "shared/assets/textures",
                  {"Image Files", "*.png *.jpg *.jpeg *.bmp *.tga *.tif",
                   "All Files", "*.*"},
                  pfd::opt::none).result();
              if (!result.empty()) {
                std::filesystem::path filePath(result[0]);
                std::string filename = filePath.filename().string();
                std::string destFolder = "shared/assets/textures/";
                std::string destPath = destFolder + filename;
                if (!std::filesystem::exists(destPath)) {
                  std::filesystem::copy_file(filePath, destPath,
                                              std::filesystem::copy_options::overwrite_existing);
                }
                mat.texturePaths[i] = destPath;
                unsigned int texID = loadTextureFromFile(destPath.c_str());
                if (texID != 0) {
                  mat.textureIDs[i] = texID;
                  mat.hasTexture[i] = true;
                } else {
                  mat.hasTexture[i] = false;
                }
              } else {
                mat.hasTexture[i] = false;
              }
            } else {
              mat.hasTexture[i] = false;
              mat.texturePaths[i] = "";
            }
          }
          if (mat.hasTexture[i] && !mat.texturePaths[i].empty()) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "%s",
                              std::filesystem::path(mat.texturePaths[i]).filename().string().c_str());
          }
          ImGui::PopID();
        }
        ImGui::TreePop();
      }

      if (ImGui::Button("Clear Override")) {
        ent->clearMaterialOverride();
      }
    }

    // Show which collection the entity belongs to
    if (ent->getCollection()) {
      ImGui::Separator();
      ImGui::Text("Collection: %s", ent->getCollection()->getName().c_str());
      if (ImGui::Button("Remove from Collection")) {
        ent->getCollection()->removeEntity(ent);
      }
    } else {
      ImGui::Separator();
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No collection (ungrouped)");
      if (!m_scene->getRootCollections().empty()) {
        ImGui::Text("Move to collection:");
        for (Collection* col : m_scene->getRootCollections()) {
          std::vector<Collection*> allCols = m_scene->getAllCollections();
          (void)col; // suppress unused warning
          break;
        }
        // Flatten all collections for a simple dropdown
        std::vector<Collection*> allCols = m_scene->getAllCollections();
        for (Collection* c : allCols) {
          if (ImGui::SmallButton(c->getName().c_str())) {
            c->addEntity(ent);
          }
          ImGui::SameLine();
        }
        ImGui::NewLine();
      }
    }
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
      m_orbitDistance = (m_orbitDistance - wheel * 2.0f < 1.0f) ? 1.0f : (m_orbitDistance - wheel * 2.0f);
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

    // FPS overlay
    ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMin().x + 8.0f,
                                       ImGui::GetItemRectMin().y + 4.0f));
    char fpsBuf[32];
    snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.0f", ImGui::GetIO().Framerate);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 0.8f));
    ImGui::TextUnformatted(fpsBuf);
    ImGui::PopStyleColor();
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

void EditorApp::renderSceneToViewport() {
  m_viewport->bind();

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Editor camera
  glm::mat4 view =
      glm::lookAt(m_editorCamPos, m_editorCamTarget, glm::vec3(0, 1, 0));
  glm::mat4 projection = glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(m_viewport->getWidth()) / m_viewport->getHeight(),
      0.1f, 10000.0f);

  drawGrid();

  for (Entity *ent : m_scene->getEntities()) {
    Shader* shader = m_standardShader.get();
    if (m_showDotMatrix) {
      shader = m_dotmatrixShader.get();
    } else if (ent->hasMaterialOverride()) {
      ShaderType st = ent->getMaterialOverridePtr()->shaderType;
      if (st == ShaderType::PBR) shader = m_pbrShader.get();
      else if (st == ShaderType::Unlit) shader = m_unlitShader.get();
    }

    shader->use();
    uploadLights(*shader, m_scene->getLights());
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

    ent->Draw(*shader);
  }

  m_viewport->unbind(
      static_cast<int>(ImGui::GetIO().DisplaySize.x),
      static_cast<int>(ImGui::GetIO().DisplaySize.y));
}

} // namespace basin
