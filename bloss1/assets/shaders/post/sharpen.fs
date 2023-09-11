#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

uniform float amount;
uniform vec2 widthHeight;

void main() {

    float neighbor = amount * -1;
    float center = amount * 4 + 1;
    vec3 color = vec3(0.0);

    color += texture(screenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 1) / widthHeight).rgb * neighbor;
    color += texture(screenTexture, vec2(gl_FragCoord.x - 1, gl_FragCoord.y + 0) / widthHeight).rgb * neighbor;
    color += texture(screenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 0) / widthHeight).rgb * center;
    color += texture(screenTexture, vec2(gl_FragCoord.x + 1, gl_FragCoord.y + 0) / widthHeight).rgb * neighbor;
    color += texture(screenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y - 1) / widthHeight).rgb * neighbor;

    fragColor = vec4(color, texture(screenTexture, fs_in.TexCoords).a);
}
