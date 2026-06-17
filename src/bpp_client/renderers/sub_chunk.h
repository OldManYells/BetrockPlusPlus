/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "world/chunk.h"

struct WorldVertex {
  glm::vec3 position;
  glm::vec3 color;
};

using SubChunkRevisions = std::array<uint64_t, 7>;

// Owns the GPU geometry and dependency state for one 16-block-tall slice.
class SubChunk {
public:
  SubChunk() = default;
  ~SubChunk();

  SubChunk(const SubChunk &) = delete;
  SubChunk &operator=(const SubChunk &) = delete;
  SubChunk(SubChunk &&other) noexcept;
  SubChunk &operator=(SubChunk &&other) noexcept;

  bool matches(const Chunk &chunk, uint8_t neighborMask,
               const SubChunkRevisions &revisions) const;
  void upload(const std::vector<WorldVertex> &vertices, const Chunk &chunk,
              uint8_t neighborMask, const SubChunkRevisions &revisions);
  void draw() const;

private:
  void release();

  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLsizei m_vertexCount = 0;
  const Chunk *m_source = nullptr;
  uint8_t m_neighborMask = 0;
  SubChunkRevisions m_revisions{};
};
