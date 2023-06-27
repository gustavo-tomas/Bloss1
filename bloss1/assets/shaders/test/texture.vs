#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
// layout(location = 5) in ivec4 boneIDs;
// layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    TexCoord = tex_coords;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
