#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

uniform int samples;
uniform float spread;
uniform float threshold;
uniform float amount;

void main() {

    float value = 0.0;
    float count = 0.0;

    vec4 result = vec4(0.0);
    vec4 screenColor = texture(screenTexture, fs_in.TexCoords);

    vec2 texSize = textureSize(screenTexture, 0).xy;

    for (int i = -samples; i <= samples; i++) {
        for (int j = -samples; j <= samples; j++) {
            vec4 color = texture(screenTexture, (gl_FragCoord.xy + (vec2(i, j) * spread)) / texSize);

            value = max(color.r, max(color.g, color.b));
            if (value < threshold) {
                color = screenColor;
            }

            result += color;
            count += 1.0;
        }
    }

    result /= count;

    fragColor = mix(screenColor, result, amount);
}
