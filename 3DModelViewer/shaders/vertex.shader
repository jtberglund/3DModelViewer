#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec3 vertexColor;

uniform mat4 mvp;

out vec3 fragColor;

void main() {
    gl_Position = mvp * vertexPos;
    fragColor = vertexColor;
}