#version 330 core

in vec3 fragColor;

uniform vec4 modelColor;

out vec4 color;

void main() {
    color = vec4(fragColor.rgb, 1.0);
}