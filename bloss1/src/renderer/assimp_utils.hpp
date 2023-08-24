#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>

static inline glm::mat4 assMatToGlmMat(aiMatrix4x4 assMatrix)
{
    glm::mat4 glmMatrix;

    glmMatrix[0][0] = assMatrix.a1; glmMatrix[1][0] = assMatrix.a2; glmMatrix[2][0] = assMatrix.a3; glmMatrix[3][0] = assMatrix.a4;
    glmMatrix[0][1] = assMatrix.b1; glmMatrix[1][1] = assMatrix.b2; glmMatrix[2][1] = assMatrix.b3; glmMatrix[3][1] = assMatrix.b4;
    glmMatrix[0][2] = assMatrix.c1; glmMatrix[1][2] = assMatrix.c2; glmMatrix[2][2] = assMatrix.c3; glmMatrix[3][2] = assMatrix.c4;
    glmMatrix[0][3] = assMatrix.d1; glmMatrix[1][3] = assMatrix.d2; glmMatrix[2][3] = assMatrix.d3; glmMatrix[3][3] = assMatrix.d4;

    return glmMatrix;
}

static inline glm::vec3 assVecToGlmVec(const aiVector3D& vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

static inline glm::quat assQuatToGlmQuat(const aiQuaternion& pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}
