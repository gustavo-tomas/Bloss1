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
    vec3 Normal;
    mat3 TBN;
    vec3 FragPos;

    vec3 viewFragPos;
    vec3 viewNormal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_PER_VERTEX = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {

    // Bone influence
    // @TODO: optimize this
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

    // Normal matrix
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    // TBN matrix
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    mat3 tbnMatrix = mat3(T, B, N); // @TODO: use second approach (light in tangent space)

    vs_out.FragPos = fragPos.xyz;
    vs_out.TexCoords = texCoords;
    vs_out.Normal = normalMatrix * normal;
    vs_out.TBN = tbnMatrix;

    // View matrices
    mat3 normalViewMatrix = transpose(inverse(mat3(view * model)));

    vs_out.viewNormal = normalViewMatrix * normal;
    vs_out.viewFragPos = (view * model * positionAfterWeights).xyz;

    gl_Position = projection * view * fragPos;
}
