/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "game_runtime.h"
#include "server_interface.h"

// Local communication layer between the client and authoritative game runtime.
class IntegratedServer final : public ServerInterface {
public:
  bool initialize(const std::string &saveName, int64_t seed) {
    return m_runtime.initialize(saveName, seed);
  }

  void setPlayerPositions(std::vector<ClientPosition> overworldPlayers,
                          std::vector<ClientPosition> netherPlayers = {}) {
    m_overworldPlayers = std::move(overworldPlayers);
    m_netherPlayers = std::move(netherPlayers);
  }

  void tick() override { m_runtime.tick(m_overworldPlayers, m_netherPlayers); }

  void stop() override { m_runtime.stop(); }

  WorldManager &getWorld(int8_t dimension = 0) override {
    return m_runtime.getWorld(dimension);
  }

  const WorldManager &getWorld(int8_t dimension = 0) const override {
    return m_runtime.getWorld(dimension);
  }

private:
  GameRuntime m_runtime;
  std::vector<ClientPosition> m_overworldPlayers;
  std::vector<ClientPosition> m_netherPlayers;
};
