#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

uniform float levels;

void main() {

    vec4 color = texture(screenTexture, fs_in.TexCoords);

    float greyscale = max(color.r, max(color.g, color.b));

    float lower = floor(greyscale * levels) / levels;
    float lowerDiff = abs(greyscale - lower);

    float upper = ceil(greyscale * levels) / levels;
    float upperDiff = abs(upper - greyscale);

    float level = lowerDiff <= upperDiff ? lower : upper;
    float adjustment = level / greyscale;

    fragColor = vec4(color.rgb * adjustment, color.a);
}
