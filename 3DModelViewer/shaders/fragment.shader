#version 330 core

in vec2 uv;
in vec3 fragPos;
in vec3 normal;

uniform vec4 modelColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float lightingEnabled;
uniform float texturingEnabled;
uniform sampler2D texSampler;

out vec4 color;

void main() {
    // Ambient lighting
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(normal);
    // lightDir is the difference vector between lightPos and fragPos
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // Texture
    if(texturingEnabled > 0.5)
        color = vec4(texture(texSampler, uv).rgb, 1.0);
    else
        color = vec4(0.8, 0.8, 0.8, 1.0);

    // Calculate lighting for this fragment
    if(lightingEnabled > 0.5) {
        vec3 specular = specularStrength * spec * lightColor;
        color = vec4((ambient + diffuse + specular) * color.rgb, 1.0f);
    }
}