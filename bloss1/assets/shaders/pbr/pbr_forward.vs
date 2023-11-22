#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in ivec4 boneIDs;
layout (location = 6) in vec4 weights;

out VS_OUT {
    vec2 TexCoords;
    mat3 TBN;
    vec3 FragPos;
} vs_out;

const int MAX_BONES = 100;
const int MAX_BONE_PER_VERTEX = 4;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {

    // Bone influence
    vec4 positionAfterWeights = vec4(0.0);
    for (int i = 0; i < MAX_BONE_PER_VERTEX; i++) {
        if (boneIDs[i] == -1)
            continue;

        if (boneIDs[i] >= MAX_BONES) {
            positionAfterWeights = vec4(position, 1.0);
            break;
        }

        vec4 localPosition = finalBonesMatrices[boneIDs[i]] * vec4(position, 1.0);
        positionAfterWeights += localPosition * weights[i];
    }

    if (positionAfterWeights == vec4(0.0)) {
        positionAfterWeights = vec4(position, 1.0);
    }

    // Position
    vec4 fragPos = model * positionAfterWeights;

    // TBN matrix
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    mat3 tbnMatrix = mat3(T, B, N);

    vs_out.FragPos = fragPos.xyz;
    vs_out.TexCoords = texCoords;
    vs_out.TBN = tbnMatrix;

    gl_Position = projection * view * fragPos;
}
