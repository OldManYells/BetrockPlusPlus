/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include "shader_program.h"
#include "sub_chunk.h"
#include "world/world.h"

class WorldRenderer {
public:
  WorldRenderer();
  ~WorldRenderer();

  WorldRenderer(const WorldRenderer &) = delete;
  WorldRenderer &operator=(const WorldRenderer &) = delete;

  void render(WorldManager &world, const glm::vec3 &cameraPosition, float yaw,
              float pitch, float aspect);

private:
  struct MeshCandidate {
    int distanceSquared;
    Int3 position;
    std::shared_ptr<Chunk> chunk;
    uint8_t neighborMask = 0;
    SubChunkRevisions revisions{};
  };

  static constexpr int MAX_MESH_BUILDS_PER_FRAME = 16;

  void syncMeshes(WorldManager &world, Int3 center);
  void removeStaleMeshes(WorldManager &world);
  std::vector<MeshCandidate> collectMeshCandidates(WorldManager &world,
                                                   Int3 center) const;
  void rebuildMesh(WorldManager &world, const MeshCandidate &candidate);
  uint8_t getNeighborMask(WorldManager &world, Int2 chunkPos) const;
  SubChunkRevisions getRevisions(WorldManager &world, const Chunk &chunk,
                                 int slice) const;

  ShaderProgram m_shader;
  GLint m_viewProjectionLocation = -1;
  std::unordered_map<Int3, SubChunk> m_meshes;
};
