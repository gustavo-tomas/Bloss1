#version 460 core

in float Height;
in float MinMaxHeight;
in vec2 TextureCoords;

out vec4 FragColor;

void main()
{
    float h = Height / (2.0 * MinMaxHeight) + 0.5; // Convert from [-m/2; m/2] -> [0; 1]
    FragColor = vec4(h, h, h, 1.0);
}
