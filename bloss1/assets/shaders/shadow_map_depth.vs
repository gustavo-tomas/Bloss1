#version 460 core

layout (location = 0) in vec3 position;
layout (location = 5) in ivec4 boneIDs;
layout (location = 6) in vec4 weights;

uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_PER_VERTEX = 4;
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

    gl_Position = model * positionAfterWeights;
}
