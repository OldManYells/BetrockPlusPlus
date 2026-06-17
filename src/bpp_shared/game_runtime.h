/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 * Copyright (c) 2026, Pixel Brush <pixelbrush.dev>
 * Copyright (c) 2026, OldManYells <OldManYells>
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "world/client_pos.h"
#include "world/storage/region_manager.h"
#include "world/storage/save_manager.h"
#include "world/world.h"

// Authoritative game state shared by every communication layer.
class GameRuntime {
public:
  GameRuntime() = default;
  ~GameRuntime();

  GameRuntime(const GameRuntime &) = delete;
  GameRuntime &operator=(const GameRuntime &) = delete;

  bool initialize(const std::string &saveName, int64_t seed);
  void prepareSpawn(int radius = 4);
  void tick(const std::vector<ClientPosition> &overworldPlayers,
            const std::vector<ClientPosition> &netherPlayers = {});
  void stop();

  Tag loadPlayerData(const std::string &playerName);
  bool savePlayerData(const std::string &playerName, Tag &playerData);

  WorldManager &getWorld(int8_t dimension = 0) {
    return dimension == -1 ? worldHell : world;
  }

  const WorldManager &getWorld(int8_t dimension = 0) const {
    return dimension == -1 ? worldHell : world;
  }

private:
  WorldManager world;
  WorldManager worldHell{true};
  SaveManager saveManager;
  RegionManager overworldRegionManager;
  RegionManager hellRegionManager;

  bool m_initialized = false;
  bool m_stopped = false;
};
