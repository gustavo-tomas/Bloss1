#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

// Textures
struct Textures {
    sampler2D screenTexture;
    sampler2D positionTexture;
};

uniform Textures textures;

uniform vec3 viewPos; // Camera position
uniform vec3 fogColor; // Color for fog calculation
uniform vec2 fogMinMax; // Range of the fog relative to the camera

void main() {

    vec3 FragPos = texture(textures.positionTexture, fs_in.TexCoords).rgb;
    vec4 color = texture(textures.screenTexture, fs_in.TexCoords);

    // Fog influence
    float dist = length(FragPos - viewPos); // Fog increases as distance to camera increases
    float fogFactor = (fogMinMax.y - dist) / (fogMinMax.y - fogMinMax.x);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Final color
    fragColor = mix(vec4(fogColor, 1.0), color, fogFactor);
}
