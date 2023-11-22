#version 460 core

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D particleTexture;
uniform vec4 color;

void main() {
    vec4 particleColor = texture(particleTexture, TexCoord);
    if (particleColor.a < 0.5)
        discard;

    FragColor = particleColor * color;
}
