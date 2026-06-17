/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 * Copyright (c) 2026, Pixel Brush <pixelbrush.dev>
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#include "game_runtime.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <unordered_set>

#include "blocks/block_properties.h"
#include "logger.h"

GameRuntime::~GameRuntime() { stop(); }

bool GameRuntime::initialize(const std::string &saveName, int64_t seed) {
  if (m_initialized)
    return !m_stopped;

  Blocks::registerAll();

  bool newSave = false;
  if (!saveManager.initialize(saveName)) {
    // initialize() also fails when another process owns this save. Do not
    // mistake that for a missing world and overwrite an existing level.
    if (std::filesystem::exists(saveName + "/level.dat")) {
      GlobalLogger().error << "Failed to acquire world save " << saveName
                           << "\n";
      return false;
    }

    GlobalLogger().info << "Creating world " << saveName << "\n";
    levelData initialData{.RandomSeed = seed, .LevelName = saveName};
    if (!saveManager.createNewWorld(initialData)) {
      GlobalLogger().error << "Failed to create world " << saveName << "\n";
      return false;
    }
    newSave = true;
  }

  if (!saveManager.loadLevelData()) {
    GlobalLogger().error << "Failed to load level data for " << saveName
                         << "\n";
    return false;
  }

  if (!overworldRegionManager.initialize(saveName + "/region") ||
      !hellRegionManager.initialize(saveName + "/DIM-1/region")) {
    GlobalLogger().error << "Failed to initialize region storage for "
                         << saveName << "\n";
    return false;
  }

  const levelData &data = saveManager.getLevelData();
  world.initWorldSeed(data.RandomSeed);
  worldHell.initWorldSeed(data.RandomSeed);
  world.elapsed_ticks = data.time;
  worldHell.elapsed_ticks = data.time;
  world.regionManager = &overworldRegionManager;
  worldHell.regionManager = &hellRegionManager;

  if (newSave)
    world.initSpawn();
  else
    world.spawnPoint = data.spawnPoint;
  worldHell.spawnPoint = world.spawnPoint;

  m_initialized = true;
  m_stopped = false;
  return true;
}

void GameRuntime::prepareSpawn(int radius) {
  if (!m_initialized || m_stopped)
    return;

  auto start = std::chrono::steady_clock::now();
  const int width = radius * 2 + 1;
  const int totalChunks = width * width;
  std::unordered_set<Int32_2> wanted;
  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dz = -radius; dz <= radius; ++dz) {
      wanted.insert(
          {(world.spawnPoint.x >> 4) + dx, (world.spawnPoint.z >> 4) + dz});
    }
  }

  for (const Int32_2 &position : wanted) {
    for (WorldManager *target : {&world, &worldHell}) {
      if (target->chunks.contains(position))
        continue;
      auto chunk = std::make_shared<Chunk>();
      chunk->spawnChunk = true;
      chunk->cpos = position;
      target->chunks.emplace(position, std::move(chunk));
    }
  }

  auto loadDimension = [&](WorldManager &target, const char *name) {
    GlobalLogger().info << "Loading spawn chunks for " << name << ": ("
                        << totalChunks << ")\n";
    int loadedChunks = 0;
    auto progressStart = std::chrono::steady_clock::now();
    while (loadedChunks < totalChunks) {
      loadedChunks = 0;
      target.pumpPipeline({});
      target.pool.wait();
      target.drainGenQueue();
      target.regionManager->iopool.wait();
      target.drainLoadQueue();
      target.populateReady();
      target.lightManager.processLightQueue(target);
      target.lightManager.processLightQueue(target);

      for (const Int32_2 &position : wanted) {
        auto found = target.chunks.find(position);
        if (found != target.chunks.end() &&
            found->second->state.load() >= ChunkState::Generated) {
          ++loadedChunks;
        }
      }

      if (std::chrono::duration<float>(std::chrono::steady_clock::now() -
                                       progressStart)
              .count() >= 1.0f) {
        const int percent = int((static_cast<float>(loadedChunks) /
                                 static_cast<float>(totalChunks)) *
                                100.0f);
        GlobalLogger().info << "Loading spawn.. " << percent << "%\n";
        progressStart = std::chrono::steady_clock::now();
      }
    }
    GlobalLogger().info << "Loading spawn.. 100%\n";
  };

  loadDimension(world, "Overworld");
  loadDimension(worldHell, "Hell");

  const float seconds =
      std::chrono::duration<float>(std::chrono::steady_clock::now() - start)
          .count();
  GlobalLogger().info << "Spawn preparation complete. (" << std::setprecision(4)
                      << seconds << "s)\n";
}

void GameRuntime::tick(const std::vector<ClientPosition> &overworldPlayers,
                       const std::vector<ClientPosition> &netherPlayers) {
  if (!m_initialized || m_stopped)
    return;

  world.tick(overworldPlayers);
  world.update(overworldPlayers);
  worldHell.tick(netherPlayers);
  worldHell.update(netherPlayers);
}

void GameRuntime::stop() {
  if (!m_initialized || m_stopped)
    return;
  m_stopped = true;

  world.shutdown();
  worldHell.shutdown();

  levelData &data = saveManager.getLevelData();
  data.RandomSeed = world.seed;
  data.spawnPoint = world.spawnPoint;
  data.time = world.elapsed_ticks;
  saveManager.saveLevelFile(data);
}

Tag GameRuntime::loadPlayerData(const std::string &playerName) {
  return saveManager.getPlayerNBT(playerName);
}

bool GameRuntime::savePlayerData(const std::string &playerName,
                                 Tag &playerData) {
  return saveManager.savePlayerNBT(playerName, playerData);
}
