#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D albedo;
uniform sampler2D arm;
uniform sampler2D tbnNormal;
uniform sampler2D depth;

void main() {

    // retrieve data from gbuffer
    vec3 FragPos = texture(position, fs_in.TexCoords).rgb;
    vec3 Normal = texture(normal, fs_in.TexCoords).rgb;
    vec4 Albedo = texture(albedo, fs_in.TexCoords);
    vec3 ARM = texture(arm, fs_in.TexCoords).rgb;
    vec3 TBNNormal = texture(tbnNormal, fs_in.TexCoords).rgb;
    float Depth = texture(depth, fs_in.TexCoords).r;

    float AO = ARM.r;
    float Roughness = ARM.g;
    float Metalness = ARM.b;

    // Final color
    vec3 finalColor = Albedo.rgb;
    fragColor = vec4(finalColor, 1.0f);
}
