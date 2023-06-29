#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord;

struct Material {
    sampler2D diffuse;   // 0
    sampler2D specular;  // 1
    sampler2D normal;    // 2
    sampler2D metalness; // 3
    sampler2D roughness; // 4
    sampler2D ao;        // 5
};

uniform Material material;

uniform vec3 color;

void main() {
    FragColor = texture(material.diffuse, TexCoord) * vec4(color, 1.0);
}
