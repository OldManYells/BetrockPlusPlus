/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#include "shader_program.h"

#include <algorithm>
#include <stdexcept>
#include <string>

ShaderProgram::ShaderProgram(const char *vertexSource,
                             const char *fragmentSource) {
  GLuint vertexShader = compile(GL_VERTEX_SHADER, vertexSource);
  GLuint fragmentShader = 0;
  try {
    fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentSource);
  } catch (...) {
    glDeleteShader(vertexShader);
    throw;
  }

  m_program = glCreateProgram();
  glAttachShader(m_program, vertexShader);
  glAttachShader(m_program, fragmentShader);
  glLinkProgram(m_program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLint success = GL_FALSE;
  glGetProgramiv(m_program, GL_LINK_STATUS, &success);
  if (success == GL_TRUE)
    return;

  GLint length = 0;
  glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);
  std::string message(static_cast<size_t>(std::max(length, 1)), '\0');
  glGetProgramInfoLog(m_program, length, nullptr, message.data());
  glDeleteProgram(m_program);
  m_program = 0;
  throw std::runtime_error("Failed to link shader program: " + message);
}

ShaderProgram::~ShaderProgram() {
  if (m_program)
    glDeleteProgram(m_program);
}

void ShaderProgram::use() const { glUseProgram(m_program); }

GLint ShaderProgram::getUniformLocation(const char *name) const {
  return glGetUniformLocation(m_program, name);
}

GLuint ShaderProgram::compile(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint success = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == GL_TRUE)
    return shader;

  GLint length = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
  std::string message(static_cast<size_t>(std::max(length, 1)), '\0');
  glGetShaderInfoLog(shader, length, nullptr, message.data());
  glDeleteShader(shader);
  throw std::runtime_error("Failed to compile shader: " + message);
}
