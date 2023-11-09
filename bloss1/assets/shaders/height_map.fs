#version 460 core

in float Height;
in float MinMaxHeight;
in vec2 TextureCoords;

out vec4 FragColor;

const uint MAX_LAYERS = 5;

uniform int layers;
uniform sampler2D textures[MAX_LAYERS];
uniform float heights[MAX_LAYERS];
uniform bool toggleGradient;

vec4 MixTextures(float H, float HeightA, sampler2D TextureA, float HeightB, sampler2D TextureB)
{
    vec4 colorA = texture(TextureA, TextureCoords);
    vec4 colorB = texture(TextureB, TextureCoords);
    
    float delta = (HeightB - HeightA);
    float fac = (H - HeightA) / delta;
    
    return mix(colorA, colorB, fac);
}

void main()
{
    float h = Height / (2.0 * MinMaxHeight) + 0.5; // Convert from [-m/2; m/2] -> [0; 1]
    vec4 color = vec4(1.0);

    for (int i = 0; i < layers; i++)
    {
        if (h < heights[0])
        {
            color = texture(textures[0], TextureCoords);
            break;
        }

        else if (h < heights[i])
        {
            color = MixTextures(h, heights[i - 1], textures[i - 1], heights[i], textures[i]);
            break;
        }
    }

    if (h >= heights[layers - 1])
    {
        color = texture(textures[layers - 1], TextureCoords);
    }

    if (toggleGradient)
    {
        FragColor = vec4(h, h, h, 1.0);
    } 
    
    else
    {
        // HDR tonemapping
        color.rgb = color.rgb / (color.rgb + vec3(1.0));

        // Gamma correction
        color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
        FragColor = color;
    }
}
