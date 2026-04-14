#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>

#include "shader.h"

std::string readFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::ate); // Open at end
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filepath << std::endl;
    return "";
  }

  size_t fileSize = file.tellg();
  std::string content(fileSize, '\0');
  file.seekg(0);
  file.read(content.data(), fileSize);
  file.close();
  return content;
}

Shader::Shader(const std::string &vertexShaderPath,
               const std::string &fragmentShaderPath) {
  std::ifstream vertexFile(vertexShaderPath);
  std::ifstream fragmentFile(fragmentShaderPath);

  // Read buffer of vertex file path and save to memeber variable
  m_vertexShaderSource = readFile(vertexShaderPath);
  m_fragmentShaderSource = readFile(fragmentShaderPath);

  // Check for common compile errors prior to opengl errors
  if (m_vertexShaderSource.size() <= 0) {
    std::cout << "ERROR::OPENGL::SHADER::VERTEX::FILE::SIZE_0" << std::endl;
  }

  if (m_fragmentShaderSource.size() <= 0) {
    std::cout << "ERROR::OPENGL::SHADER::FRAGMENT::FILE::SIZE_0" << std::endl;
  }

  if (m_vertexShaderSource.size() > 0 &&
      (m_vertexShaderSource[0] == ' ' || m_vertexShaderSource[0] == '\n' ||
       m_vertexShaderSource[0] == '\r' || m_vertexShaderSource[0] == '\t')) {
    m_vertexShaderSource.erase(0, 1);
  }

  if (m_fragmentShaderSource.size() > 0 &&
      (m_fragmentShaderSource[0] == ' ' || m_fragmentShaderSource[0] == '\n' ||
       m_fragmentShaderSource[0] == '\r' ||
       m_fragmentShaderSource[0] == '\t')) {
    m_fragmentShaderSource.erase(0, 1);
  }
  // Compile shaders from source
  CompileShaders();

  // Create program from compiled shaders
  CreateShaderProgram();
}

Shader::~Shader() { CleanupShaders(); }

void Shader::use() { glUseProgram(m_shaderProgram); }

void Shader::CompileShaders() {
  int success;
  char infoLog[512];

  m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
  m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *vSrc = m_vertexShaderSource.c_str();
  const char *fSrc = m_fragmentShaderSource.c_str();

  glShaderSource(m_vertexShader, 1, &vSrc, NULL);
  glCompileShader(m_vertexShader);
  glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(m_vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::OPENGL::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  glShaderSource(m_fragmentShader, 1, &fSrc, NULL);
  glCompileShader(m_fragmentShader);
  glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(m_fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::OPENGL::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
}

void Shader::CreateShaderProgram() {
  int success;
  char infoLog[512];

  m_shaderProgram = glCreateProgram();
  glAttachShader(m_shaderProgram, m_vertexShader);
  glAttachShader(m_shaderProgram, m_fragmentShader);
  glLinkProgram(m_shaderProgram);
  glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

  if (!success) {
    glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::OPENGL::PROGRAM::LINK_FAIL\n" << infoLog << std::endl;
  }
}

void Shader::CleanupShaders() {
  glDeleteShader(m_vertexShader);
  glDeleteShader(m_fragmentShader);
}

void Shader::setUniform(const std::string &name, const glm::mat4 &value) {
  GLuint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::mat3 &value) {
  GLuint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
  glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::vec4 &value) {
  GLuint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
  glUniform4fv(loc, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::vec3 &value) {
  GLuint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
  glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, float value) {
  GLuint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
  glUniform1f(loc, value);
}

void Shader::setUniform(const std::string &name, int value) {
  GLuint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
  glUniform1i(loc, value);
}
