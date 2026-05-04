#ifndef BASIN_EDITOR_APP_H
#define BASIN_EDITOR_APP_H

#include "basin/engine.h"
#include "basin/scene/scene.h"
#include "basin/scene/collection.h"
#include "basin/renderer/shader.h"
#include "basin/renderer/text_renderer.h"
#include "viewport.h"
#include <memory>

namespace basin {

class EditorApp : public Application {
public:
  void onInit(Window &window) override;
  void onUpdate(float deltaTime, Window &window) override;
  void onRender() override;
  void onShutdown() override;

private:
  void drawMenuBar();
  void drawSceneHierarchy();
  void drawInspector();
  void drawViewport();
  void renderSceneToViewport();
  void drawCollectionTree(const std::vector<Collection*>& collections, int& nodeIndex);
  void createGrid();
  void drawGrid();

  std::unique_ptr<Scene> m_scene;
  std::unique_ptr<Viewport> m_viewport;
  std::unique_ptr<Shader> m_standardShader;
  std::unique_ptr<Shader> m_dotmatrixShader;
  std::unique_ptr<Shader> m_pbrShader;
  std::unique_ptr<Shader> m_unlitShader;
  Shader *m_activeShader = nullptr;
  std::unique_ptr<TextRenderer> m_textRenderer;

  // Grid
  GLuint m_gridVAO = 0;
  GLuint m_gridVBO = 0;
  int m_gridVertexCount = 0;

  int m_selectedEntity = -1;
  int m_selectedLight = -1;
  bool m_selectionIsLight = false;
  Collection* m_selectedCollection = nullptr;

  glm::vec3 m_collectionDeltaPos = glm::vec3(0.0f);
  bool m_useDotMatrix = true;
  bool m_showDotMatrix = false;
  bool m_confirmDeleteCollection = false;

  // Camera for editor viewport
  glm::vec3 m_editorCamPos = glm::vec3(0.0f, 10.0f, 20.0f);
  glm::vec3 m_editorCamTarget = glm::vec3(0.0f, 0.0f, 0.0f);

  // Editor camera control state
  glm::dvec2 m_lastMousePos = glm::dvec2(0.0);
  bool m_mmbDragging = false;
  float m_orbitDistance = 20.0f;
  float m_orbitYaw = -90.0f;
  float m_orbitPitch = 30.0f;
};

} // namespace basin

#endif
