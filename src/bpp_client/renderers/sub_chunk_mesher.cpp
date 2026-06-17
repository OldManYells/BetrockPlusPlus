/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#include "sub_chunk_mesher.h"

#include <algorithm>

#include "blocks/faces.h"

namespace {
bool shouldRenderFace(BlockType block, BlockType neighbor) {
  if (neighbor == BLOCK_AIR)
    return true;
  if (neighbor == block)
    return false;
  return !Blocks::blockProperties[static_cast<uint8_t>(neighbor)].isOpaqueCube;
}
} // namespace

std::vector<WorldVertex> SubChunkMesher::build(WorldManager &world,
                                               const Chunk &chunk, int slice) {
  std::vector<WorldVertex> vertices;
  vertices.reserve(4'000);

  const int baseX = chunk.cpos.x * CHUNK_WIDTH;
  const int baseZ = chunk.cpos.z * CHUNK_WIDTH;
  const int minY = slice * SUB_CHUNK_SIZE;
  const int maxY = std::min(minY + SUB_CHUNK_SIZE, CHUNK_HEIGHT);
  for (int y = minY; y < maxY; ++y) {
    for (int z = 0; z < CHUNK_WIDTH; ++z) {
      for (int x = 0; x < CHUNK_WIDTH; ++x) {
        const Int3 localPosition{x, y, z};
        const BlockType block = chunk.getBlock(localPosition);
        if (block == BLOCK_AIR)
          continue;

        const Blocks::BlockColor &baseColor =
            Blocks::blockProperties[static_cast<uint8_t>(block)].baseColor;
        const Int3 worldPosition{baseX + x, y, baseZ + z};
        const float light = std::max(
            0.28f,
            static_cast<float>(chunk.getBlockLightValue(localPosition, 0)) /
                15.0f);
        for (const BlockFace &face : BLOCK_FACES) {
          if (!shouldRenderFace(block,
                                world.getBlockId(worldPosition + face.normal)))
            continue;

          const glm::vec3 color{baseColor.red * face.shade * light,
                                baseColor.green * face.shade * light,
                                baseColor.blue * face.shade * light};
          const glm::vec3 offset{static_cast<float>(worldPosition.x),
                                 static_cast<float>(worldPosition.y),
                                 static_cast<float>(worldPosition.z)};
          for (const glm::vec3 &vertex : face.vertices)
            vertices.push_back({vertex + offset, color});
        }
      }
    }
  }

  return vertices;
}
