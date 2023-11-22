#version 460 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D frame;

void main() {
    FragColor = texture(frame, TexCoords);
}
