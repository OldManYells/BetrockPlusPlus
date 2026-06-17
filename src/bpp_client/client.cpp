/*
 * Copyright (c) 2026, Aidan <JcbbcEnjoyer>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <glm/glm.hpp>
#include "client.h"

extern std::atomic<bool> shutdownRequested;

// This m_window size seems really random but its the size beta uses
Client::Client() : m_window(854, 480, "Betrock++") {
    // Set up shared context before registering any callbacks
    m_ctx = { &m_window, &m_input };
    glfwSetWindowUserPointer(m_window.getHandle(), &m_ctx);

    m_input.init(m_window.getHandle());
    m_window.initCallbacks(m_window.getHandle());

    m_window.setCursorLocked(true);
    m_window.setVsync(true);

    if (!m_server.initialize("client_world", 8675309))
        throw std::runtime_error("Failed to initialize integrated server");

    const Int3 spawn = m_server.getWorld().spawnPoint;
    m_singlePlayerPos.pos = {
        static_cast<double>(spawn.x) + 0.5,
        static_cast<double>(spawn.y) + 18.0,
        static_cast<double>(spawn.z) + 0.5
    };
    m_singlePlayerPos.viewDistanceOverride = 4;
    m_previousPos = m_singlePlayerPos.pos;

    GlobalLogger().info << "Client initialized\n";
}

Client::~Client() {
    m_server.stop();
}

void Client::tick() {
    m_previousPos = m_singlePlayerPos.pos;

    if (m_input.isKeyPressed(GLFW_KEY_ESCAPE)) {
        m_window.requestClose();
        return;
    }

    m_yaw += m_input.mouseDeltaX() * MOUSE_SENSITIVITY;
    m_pitch = std::clamp(
        m_pitch + m_input.mouseDeltaY() * MOUSE_SENSITIVITY, -89.0f, 89.0f);

    const float yawRadians = glm::radians(m_yaw);
    glm::vec3 forward{ std::cos(yawRadians), 0.0f, std::sin(yawRadians) };
    glm::vec3 right{ -forward.z, 0.0f, forward.x };
    glm::vec3 movement{ 0.0f };

    if (m_input.isKeyHeld(GLFW_KEY_W)) movement += forward;
    if (m_input.isKeyHeld(GLFW_KEY_S)) movement -= forward;
    if (m_input.isKeyHeld(GLFW_KEY_D)) movement += right;
    if (m_input.isKeyHeld(GLFW_KEY_A)) movement -= right;
    if (m_input.isKeyHeld(GLFW_KEY_SPACE)) movement.y += 1.0f;
    if (m_input.isKeyHeld(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

    if (glm::dot(movement, movement) > 0.0f) {
        movement = glm::normalize(movement);
        const float speed = m_input.isKeyHeld(GLFW_KEY_LEFT_CONTROL) ? 36.0f : 12.0f;
        movement *= speed * TICK_DELTA;
        m_singlePlayerPos.pos.x += movement.x;
        m_singlePlayerPos.pos.y += movement.y;
        m_singlePlayerPos.pos.z += movement.z;
    }

    m_server.setPlayerPositions({ m_singlePlayerPos });
    m_server.tick();
}

void Client::render(float partial_tick) {
    const float inversePartial = 1.0f - partial_tick;
    const glm::vec3 cameraPosition{
        static_cast<float>(m_previousPos.x * inversePartial + m_singlePlayerPos.pos.x * partial_tick),
        static_cast<float>(m_previousPos.y * inversePartial + m_singlePlayerPos.pos.y * partial_tick),
        static_cast<float>(m_previousPos.z * inversePartial + m_singlePlayerPos.pos.z * partial_tick)
    };

    glClearColor(0.46f, 0.68f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_worldRenderer.render(
        m_server.getWorld(), cameraPosition, m_yaw, m_pitch, m_window.getAspect());
}

int Client::run() {
    float lastTime = float(glfwGetTime());

    while (!m_window.shouldClose() && !shutdownRequested.load()) {
        int   ticks_ran = 0;
        float now = float(glfwGetTime());
        float delta = now - lastTime;
        lastTime = now;
        m_accumulator += delta;

        m_window.pollEvents();

        // Run ticks until caught up, but cap to avoid spiraling on slow frames
        while (m_accumulator >= TICK_DELTA && ticks_ran < MAX_TICKS_PER_FRAME) {
            m_input.drainEvents();
            tick();
            m_input.flush();
            m_accumulator -= TICK_DELTA;
            ticks_ran++;
        }

        // Discard leftover time if we hit the cap
        if (ticks_ran == MAX_TICKS_PER_FRAME)
            m_accumulator = 0.0f;

        render(m_accumulator / TICK_DELTA);
        m_window.swapBuffers();
    }
    shutdownRequested.store(false);
    return 0;
}
