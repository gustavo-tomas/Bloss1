#version 460 core

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform vec3 color;
uniform sampler2D simple_texture;

void main() {
    FragColor = texture(simple_texture, TexCoord) * vec4(color, 1.0);
}
