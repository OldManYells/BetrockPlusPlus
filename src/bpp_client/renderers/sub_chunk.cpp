/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#include "sub_chunk.h"

#include <cstddef>
#include <utility>

SubChunk::~SubChunk() { release(); }

SubChunk::SubChunk(SubChunk &&other) noexcept { *this = std::move(other); }

SubChunk &SubChunk::operator=(SubChunk &&other) noexcept {
  if (this == &other)
    return *this;

  release();
  m_vao = std::exchange(other.m_vao, 0);
  m_vbo = std::exchange(other.m_vbo, 0);
  m_vertexCount = std::exchange(other.m_vertexCount, 0);
  m_source = std::exchange(other.m_source, nullptr);
  m_neighborMask = other.m_neighborMask;
  m_revisions = other.m_revisions;
  return *this;
}

bool SubChunk::matches(const Chunk &chunk, uint8_t neighborMask,
                       const SubChunkRevisions &revisions) const {
  return m_source == &chunk && m_neighborMask == neighborMask &&
         m_revisions == revisions;
}

void SubChunk::upload(const std::vector<WorldVertex> &vertices,
                      const Chunk &chunk, uint8_t neighborMask,
                      const SubChunkRevisions &revisions) {
  release();
  m_source = &chunk;
  m_neighborMask = neighborMask;
  m_revisions = revisions;
  m_vertexCount = static_cast<GLsizei>(vertices.size());
  if (vertices.empty())
    return;

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(vertices.size() * sizeof(WorldVertex)),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
      reinterpret_cast<void *>(offsetof(WorldVertex, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                        reinterpret_cast<void *>(offsetof(WorldVertex, color)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);
}

void SubChunk::draw() const {
  if (m_vertexCount == 0)
    return;

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
}

void SubChunk::release() {
  if (m_vao)
    glDeleteVertexArrays(1, &m_vao);
  if (m_vbo)
    glDeleteBuffers(1, &m_vbo);
  m_vao = 0;
  m_vbo = 0;
  m_vertexCount = 0;
}
