/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <vector>

#include "sub_chunk.h"
#include "world/world.h"

namespace SubChunkMesher {
std::vector<WorldVertex> build(WorldManager &world, const Chunk &chunk,
                               int slice);
}
