#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gARM;
layout (location = 4) out vec3 gTBNNormal;
layout (location = 5) out float gDepth;
layout (location = 6) out vec3 gViewFragPos;
layout (location = 7) out vec3 gViewNormal;

in VS_OUT {
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
    vec3 FragPos;

    vec3 viewFragPos;
    vec3 viewNormal;
} fs_in;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    sampler2D metalness;
    sampler2D roughness;
    sampler2D ao;
};

uniform Material material;

void main() {    
    // Store data in the gbuffer textures
    gPosition = fs_in.FragPos;
    gNormal = normalize(fs_in.Normal);
    gAlbedo = texture(material.diffuse, fs_in.TexCoords);
    gARM = texture(material.metalness, fs_in.TexCoords).rgb; // ARM = AO, Roughness, Metalness (1 for each RGB channel)
    gDepth = gl_FragCoord.z;

    vec3 normal = texture(material.normal, fs_in.TexCoords).rgb;
    gTBNNormal = normalize(fs_in.TBN * (normal * 2.0 - 1.0)); // range [0,1] -> [-1,1]

    gViewFragPos = fs_in.viewFragPos;
    gViewNormal = normalize(fs_in.viewNormal);
}
