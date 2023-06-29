#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
// layout(location = 5) in ivec4 boneIDs;
// layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out VS_OUT {
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
    vec3 FragPos;
    vec4 FragPosLightSpace;
} vs_out;

void main() {

    // TBN matrix
    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));

    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.TexCoords = tex_coords;
    vs_out.Normal = mat3(transpose(inverse(model))) * normal;
    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(position, 1.0);
}
