#version 330 core

in vec2 uv;

uniform vec4 modelColor;
uniform sampler2D texSampler;

out vec4 color;

void main() {
    //color = vec4(fragColor.rgb, 1.0);
    color = vec4(texture(texSampler, uv).rgb, 1.0);
    //color = texture2D(texSampler, uv.st);
    //color = vec4(0.5, 0.5, 0.0, 1.0);

}