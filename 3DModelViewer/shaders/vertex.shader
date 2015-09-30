#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 model;

out vec2 uv;
out vec3 fragPos;
out vec3 normal;

void main() {
    gl_Position = mvp * vec4(vertexPos, 1.0f);
    uv = vertexUV;
    fragPos = vec3(model * vec4(vertexPos, 1.0f));
    normal = vertexNormal;
}