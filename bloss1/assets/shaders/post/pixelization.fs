#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

uniform int pixelSize;
uniform vec2 widthHeight;

void main() {

    float x = int(gl_FragCoord.x) % pixelSize;
    float y = int(gl_FragCoord.y) % pixelSize;

    x = floor(pixelSize / 2.0) - x;
    y = floor(pixelSize / 2.0) - y;

    x = gl_FragCoord.x + x;
    y = gl_FragCoord.y + y;

    fragColor = texture(screenTexture, vec2(x, y) / widthHeight);
}
