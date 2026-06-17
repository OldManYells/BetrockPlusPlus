/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
*/

#pragma once
#include "glfw_context.h"
#include "inputs.h"
#include "window.h"
#include "renderers/world_renderer.h"
#include "integrated_server.h"

class Client {
public:
    Client();
    ~Client();
    int run();
private:
    static constexpr float MOUSE_SENSITIVITY = 0.12f;
    static constexpr float TICK_DELTA = 1.0f / 20.0f;
    static constexpr int   MAX_TICKS_PER_FRAME = 10;

    void tick();
    void render([[maybe_unused]] float partial_tick);

    Window      m_window;
    Input       m_input;
    GlfwContext m_ctx;
    ClientPosition m_singlePlayerPos{};
    Vec3        m_previousPos{};
    IntegratedServer m_server;
    WorldRenderer m_worldRenderer;
    float       m_yaw = -90.0f;
    float       m_pitch = -20.0f;
    float       m_accumulator = 0.0f;
};
