/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#include "world_renderer.h"

#include <algorithm>
#include <array>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/world.h"
#include "sub_chunk_mesher.h"

namespace {
bool isRenderable(const Chunk &chunk) {
  const ChunkState state = chunk.state.load(std::memory_order_acquire);
  return state >= ChunkState::Generated && state != ChunkState::Unloading;
}

Int3 subChunkAt(const glm::vec3 &position) {
  return {static_cast<int>(std::floor(position.x)) >> 4,
          static_cast<int>(std::floor(position.y / SUB_CHUNK_SIZE)),
          static_cast<int>(std::floor(position.z)) >> 4};
}

glm::mat4 viewProjection(const glm::vec3 &cameraPosition, float yaw,
                         float pitch, float aspect) {
  const float yawRadians = glm::radians(yaw);
  const float pitchRadians = glm::radians(pitch);
  const glm::vec3 forward = glm::normalize(glm::vec3{
      std::cos(yawRadians) * std::cos(pitchRadians), std::sin(pitchRadians),
      std::sin(yawRadians) * std::cos(pitchRadians)});
  const glm::mat4 projection = glm::perspective(
      glm::radians(70.0f), std::max(aspect, 0.01f), 0.05f, 512.0f);
  const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + forward,
                                     glm::vec3{0.0f, 1.0f, 0.0f});
  return projection * view;
}
} // namespace

WorldRenderer::WorldRenderer()
    : m_shader(WorldShader::VERTEX, WorldShader::FRAGMENT) {
  m_viewProjectionLocation = m_shader.getUniformLocation("viewProjection");
}

WorldRenderer::~WorldRenderer() = default;

void WorldRenderer::render(WorldManager &world, const glm::vec3 &cameraPosition,
                           float yaw, float pitch, float aspect) {
  syncMeshes(world, subChunkAt(cameraPosition));

  const glm::mat4 cameraMatrix =
      viewProjection(cameraPosition, yaw, pitch, aspect);
  m_shader.use();
  glUniformMatrix4fv(m_viewProjectionLocation, 1, GL_FALSE,
                     glm::value_ptr(cameraMatrix));

  for (const auto &[position, mesh] : m_meshes) {
    (void)position;
    mesh.draw();
  }
  glBindVertexArray(0);
}

void WorldRenderer::syncMeshes(WorldManager &world, Int3 center) {
  removeStaleMeshes(world);
  std::vector<MeshCandidate> candidates = collectMeshCandidates(world, center);
  std::sort(candidates.begin(), candidates.end(),
            [](const MeshCandidate &left, const MeshCandidate &right) {
              return left.distanceSquared < right.distanceSquared;
            });

  const size_t buildCount = std::min(
      candidates.size(), static_cast<size_t>(MAX_MESH_BUILDS_PER_FRAME));
  for (size_t index = 0; index < buildCount; ++index)
    rebuildMesh(world, candidates[index]);
}

void WorldRenderer::removeStaleMeshes(WorldManager &world) {
  for (auto mesh = m_meshes.begin(); mesh != m_meshes.end();) {
    auto chunk = world.getChunk({mesh->first.x, mesh->first.z});
    if (!chunk || !isRenderable(*chunk))
      mesh = m_meshes.erase(mesh);
    else
      ++mesh;
  }
}

std::vector<WorldRenderer::MeshCandidate>
WorldRenderer::collectMeshCandidates(WorldManager &world, Int3 center) const {
  std::vector<MeshCandidate> candidates;
  candidates.reserve(world.chunks.size() * Chunk::SUB_CHUNK_COUNT);

  for (const auto &[position, chunk] : world.chunks) {
    if (!chunk || !isRenderable(*chunk))
      continue;

    const uint8_t neighborMask = getNeighborMask(world, position);
    const int dx = position.x - center.x;
    const int dz = position.z - center.z;
    for (int slice = 0; slice < Chunk::SUB_CHUNK_COUNT; ++slice) {
      const Int3 subChunkPosition{position.x, slice, position.z};
      const SubChunkRevisions revisions = getRevisions(world, *chunk, slice);
      auto existing = m_meshes.find(subChunkPosition);
      if (existing != m_meshes.end() &&
          existing->second.matches(*chunk, neighborMask, revisions))
        continue;

      const int dy = slice - center.y;
      candidates.push_back({dx * dx + dy * dy + dz * dz, subChunkPosition,
                            chunk, neighborMask, revisions});
    }
  }

  return candidates;
}

void WorldRenderer::rebuildMesh(WorldManager &world,
                                const MeshCandidate &candidate) {
  std::vector<WorldVertex> vertices =
      SubChunkMesher::build(world, *candidate.chunk, candidate.position.y);
  m_meshes[candidate.position].upload(
      vertices, *candidate.chunk, candidate.neighborMask, candidate.revisions);
}

uint8_t WorldRenderer::getNeighborMask(WorldManager &world,
                                       Int2 chunkPos) const {
  constexpr std::array<Int2, 4> OFFSETS{{
      {-1, 0},
      {1, 0},
      {0, -1},
      {0, 1},
  }};
  uint8_t mask = 0;
  for (size_t index = 0; index < OFFSETS.size(); ++index) {
    auto neighbor = world.getChunk(chunkPos + OFFSETS[index]);
    if (neighbor && isRenderable(*neighbor))
      mask |= static_cast<uint8_t>(1u << index);
  }
  return mask;
}

SubChunkRevisions WorldRenderer::getRevisions(WorldManager &world,
                                              const Chunk &chunk,
                                              int slice) const {
  SubChunkRevisions revisions{};
  revisions[0] = chunk.getMeshRevision(slice);
  if (slice > 0)
    revisions[1] = chunk.getMeshRevision(slice - 1);
  if (slice + 1 < Chunk::SUB_CHUNK_COUNT)
    revisions[2] = chunk.getMeshRevision(slice + 1);

  constexpr std::array<Int2, 4> OFFSETS{{
      {-1, 0},
      {1, 0},
      {0, -1},
      {0, 1},
  }};
  for (size_t index = 0; index < OFFSETS.size(); ++index) {
    auto neighbor = world.getChunk(chunk.cpos + OFFSETS[index]);
    if (neighbor && isRenderable(*neighbor))
      revisions[index + 3] = neighbor->getMeshRevision(slice);
  }
  return revisions;
}
