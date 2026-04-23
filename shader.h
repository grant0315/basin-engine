#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
  Shader(const std::string &vertexShaderPath,
         const std::string &fragmentShaderPath);
  ~Shader();

  void use();

  void setUniform(const std::string &name, const glm::mat4 &value);
  void setUniform(const std::string &name, const glm::mat3 &value);
  void setUniform(const std::string &name, const glm::vec4 &value);
  void setUniform(const std::string &name, const glm::vec3 &value);
  void setUniform(const std::string &name, float value);
  void setUniform(const std::string &name, int value);

private:
  void CompileShaders();
  void CreateShaderProgram();
  void CleanupShaders();

  GLint getUniformLocation(const std::string &name);

  std::string m_vertexShaderSource;
  std::string m_fragmentShaderSource;
  unsigned int m_vertexShader;
  unsigned int m_fragmentShader;
  unsigned int m_shaderProgram;
  std::unordered_map<std::string, GLint> m_uniformLocationCache;
};

#endif
