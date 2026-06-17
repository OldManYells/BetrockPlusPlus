/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <glad/glad.h>

class ShaderProgram {
public:
  ShaderProgram(const char *vertexSource, const char *fragmentSource);
  ~ShaderProgram();

  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram &operator=(const ShaderProgram &) = delete;

  void use() const;
  GLint getUniformLocation(const char *name) const;

private:
  static GLuint compile(GLenum type, const char *source);

  GLuint m_program = 0;
};
