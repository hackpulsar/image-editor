#pragma once

#include <string>

#define GLSL(src) "#version 330\n" #src

static std::string vertexShaderSrc = GLSL(
    layout(location = 0) in vec4 position;
    layout(location = 1) in vec2 texCoord;

    out vec2 v_TexCoord;

    uniform mat4 u_MVP;

    void main()
    {
        gl_Position = u_MVP * position;
        v_TexCoord = texCoord;
    }
);

static std::string fragmentShaderSrc = GLSL(
    layout(location = 0) out vec4 color;

    in vec2 v_TexCoord;

    uniform sampler2D u_Texture;

    void main()
    {
        vec4 texColor = texture(u_Texture, v_TexCoord);
        color = texColor;
    }
);
