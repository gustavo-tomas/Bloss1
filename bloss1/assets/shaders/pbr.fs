#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

struct Textures {
    sampler2D position;
    sampler2D normal;
    sampler2D albedo;
    sampler2D arm;
    sampler2D tbnNormal;
    sampler2D depth;
};

uniform Textures textures;

void main() {

    // retrieve data from gbuffer
    vec3 FragPos = texture(textures.position, fs_in.TexCoords).rgb;
    vec3 Normal = texture(textures.normal, fs_in.TexCoords).rgb;
    vec4 Albedo = texture(textures.albedo, fs_in.TexCoords);
    vec3 ARM = texture(textures.arm, fs_in.TexCoords).rgb;
    vec3 TBNNormal = texture(textures.tbnNormal, fs_in.TexCoords).rgb;
    float Depth = texture(textures.depth, fs_in.TexCoords).r;

    float AO = ARM.r;
    float Roughness = ARM.g;
    float Metalness = ARM.b;

    // Final color
    vec3 finalColor = Albedo.rgb;
    fragColor = vec4(finalColor, 1.0f);
}
