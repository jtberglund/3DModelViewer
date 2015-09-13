#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec2 vertexUV;

uniform mat4 mvp;

out vec2 uv;

void main() {
    gl_Position = mvp * vertexPos;
    uv = vertexUV;
}