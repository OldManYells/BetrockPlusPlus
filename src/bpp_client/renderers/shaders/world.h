/*
 * Copyright (c) 2026, OldManYells <OldManYells>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 *
 */

#pragma once

namespace WorldShader {
inline constexpr const char *VERTEX = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;

    uniform mat4 viewProjection;
    out vec3 vertexColor;

    void main() {
        vertexColor = color;
        gl_Position = viewProjection * vec4(position, 1.0);
    }
)";

inline constexpr const char *FRAGMENT = R"(
    #version 330 core
    in vec3 vertexColor;
    out vec4 fragmentColor;

    void main() {
        fragmentColor = vec4(vertexColor, 1.0);
    }
)";
} // namespace WorldShader
