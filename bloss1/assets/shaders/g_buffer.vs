#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

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

void main() {

    // Position
    vec4 fragPos = model * vec4(position, 1.0f);

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
    vs_out.viewFragPos = (view * model * vec4(position, 1.0f)).xyz;

    gl_Position = projection * view * fragPos;
}
