/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <array>

#include <glm/vec3.hpp>

#include <numeric_structs.h>

// Simple helper for generating the faces of blocks
// this most likely will be replaces with something more dynamic
// because the cube god did not make all voxels equally

enum class BlockFaceDirection { East, West, Up, Down, South, North };

struct BlockFace {
  BlockFaceDirection direction;
  Int3 normal;
  std::array<glm::vec3, 6> vertices;
  float shade;
};

// This function attempts to make face definitions a bit more friendlier to the
// eye... arguably
constexpr BlockFace makeBlockFace(BlockFaceDirection direction, Int3 normal,
                                  float shade, glm::vec3 first,
                                  glm::vec3 second, glm::vec3 third,
                                  glm::vec3 fourth) {
  return {
      direction, normal, {first, second, third, first, third, fourth}, shade};
}

// Until we have Voxel AO as beta should, we're using the old different shading
// per face.
inline constexpr std::array<BlockFace, 6> BLOCK_FACES{
    makeBlockFace(BlockFaceDirection::East, {1, 0, 0}, 0.80f, {1, 0, 0},
                  {1, 1, 0}, {1, 1, 1}, {1, 0, 1}),
    makeBlockFace(BlockFaceDirection::West, {-1, 0, 0}, 0.80f, {0, 0, 1},
                  {0, 1, 1}, {0, 1, 0}, {0, 0, 0}),
    makeBlockFace(BlockFaceDirection::Up, {0, 1, 0}, 1.00f, {0, 1, 0},
                  {0, 1, 1}, {1, 1, 1}, {1, 1, 0}),
    makeBlockFace(BlockFaceDirection::Down, {0, -1, 0}, 0.50f, {0, 0, 1},
                  {0, 0, 0}, {1, 0, 0}, {1, 0, 1}),
    makeBlockFace(BlockFaceDirection::South, {0, 0, 1}, 0.70f, {1, 0, 1},
                  {1, 1, 1}, {0, 1, 1}, {0, 0, 1}),
    makeBlockFace(BlockFaceDirection::North, {0, 0, -1}, 0.70f, {0, 0, 0},
                  {0, 1, 0}, {1, 1, 0}, {1, 0, 0})};
