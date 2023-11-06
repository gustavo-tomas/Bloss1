#version 460 core

layout (location = 0) in vec3 position;

out float Height;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Height = position.y;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
