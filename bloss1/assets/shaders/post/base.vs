#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out VS_OUT {
    vec2 TexCoords;
} vs_out;

void main() {
    gl_Position = vec4(position, 1.0);
    vs_out.TexCoords = texCoords;
}
