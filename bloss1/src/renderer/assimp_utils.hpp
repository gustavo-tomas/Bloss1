#pragma once

#include "math/math.hpp"

#include "assimp/Importer.hpp"

namespace bls
{
    static inline mat4 ass_mat_to_glm_mat(aiMatrix4x4 ass_matrix)
    {
        mat4 glm_matrix;

        glm_matrix[0][0] = ass_matrix.a1; glm_matrix[1][0] = ass_matrix.a2; glm_matrix[2][0] = ass_matrix.a3; glm_matrix[3][0] = ass_matrix.a4;
        glm_matrix[0][1] = ass_matrix.b1; glm_matrix[1][1] = ass_matrix.b2; glm_matrix[2][1] = ass_matrix.b3; glm_matrix[3][1] = ass_matrix.b4;
        glm_matrix[0][2] = ass_matrix.c1; glm_matrix[1][2] = ass_matrix.c2; glm_matrix[2][2] = ass_matrix.c3; glm_matrix[3][2] = ass_matrix.c4;
        glm_matrix[0][3] = ass_matrix.d1; glm_matrix[1][3] = ass_matrix.d2; glm_matrix[2][3] = ass_matrix.d3; glm_matrix[3][3] = ass_matrix.d4;

        return glm_matrix;
    }

    static inline vec3 ass_vec_to_glm_vec(const aiVector3D& vec)
    {
        return vec3(vec.x, vec.y, vec.z);
    }

    static inline quat ass_quat_to_glm_quat(const aiQuaternion& orientation)
    {
        return quat(orientation.w, orientation.x, orientation.y, orientation.z);
    }
};
