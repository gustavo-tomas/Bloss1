#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D fbo_texture;

void main() {
    fragColor = texture(fbo_texture, fs_in.TexCoords);
}
