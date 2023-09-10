#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

void main() {
    fragColor = texture(screenTexture, fs_in.TexCoords);
}
