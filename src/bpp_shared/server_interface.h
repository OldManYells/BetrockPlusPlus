/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

#include <cstdint>

#include "world/world.h"

// Common contract to be implemented by both integrated and dedicated server
class ServerInterface {
public:
  virtual ~ServerInterface() = default;

  virtual void tick() = 0;
  virtual void stop() = 0;

  virtual WorldManager &getWorld(int8_t dimension = 0) = 0;
  virtual const WorldManager &getWorld(int8_t dimension = 0) const = 0;
};
