#version 460 core

in float Height;
in float MinMaxHeight;
in vec2 TextureCoords;
in vec3 FragPos;

out vec4 FragColor;

const uint MAX_LAYERS = 5;

uniform int layers;
uniform sampler2D textures[MAX_LAYERS];
uniform float heights[MAX_LAYERS];
uniform bool toggleGradient;

uniform mat4 model;
uniform mat4 view;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight dirLight;

vec4 MixTextures(float H, float HeightA, sampler2D TextureA, float HeightB, sampler2D TextureB)
{
    vec4 colorA = texture(TextureA, TextureCoords);
    vec4 colorB = texture(TextureB, TextureCoords);
    
    float delta = (HeightB - HeightA);
    float fac = (H - HeightA) / delta;
    
    return mix(colorA, colorB, fac);
}

// Phong lighting
vec3 calcDirLight(DirLight dirLight, vec3 normalizedNormal, vec3 normalizedViewDir, vec3 materialColor);

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
        return;
    } 
    
    // HDR tonemapping
    color.rgb = color.rgb / (color.rgb + vec3(1.0));

    // Gamma correction
    color.rgb = pow(color.rgb, vec3(1.0 / 2.2));

    vec3 viewPos = view[3].xyz;

    vec3 x = dFdx(FragPos);
    vec3 y = dFdy(FragPos);
    vec3 normal = inverse(mat3(model)) * normalize(cross(x, y));
    vec3 normalizedViewDir = normalize(viewPos - FragPos);

    vec3 dirLightColor = calcDirLight(dirLight, normal, normalizedViewDir, color.rgb);
    FragColor = vec4(dirLightColor, color.a);
}

float calculateSpecularImpact(vec3 normalizedLightDir, vec3 normalizedViewDir, vec3 normalizedNormal) {
    vec3 normalizedHalfwayDir = normalize(normalizedLightDir + normalizedViewDir);
    float specularImpact = pow(max(dot(normalizedNormal, normalizedHalfwayDir), 0.0), 32 * 2.0);

    return specularImpact;
}

float calculateDiffuseImpact(vec3 normalizedNormal, vec3 normalizedLightDir) {
    return max(dot(normalizedNormal, normalizedLightDir), 0.0);
}

vec3 calcDirLight(DirLight dirLight, vec3 normalizedNormal, vec3 normalizedViewDir, vec3 materialColor) {
    vec3 normalizedLightDir = normalize(-dirLight.direction);
    // vec3 normalizedLightDir = normalize(dirLight.position - fs_in.FragPos);

    float diffuseImpact = calculateDiffuseImpact(normalizedNormal, normalizedLightDir);
    float specularImpact = calculateSpecularImpact(normalizedLightDir, normalizedViewDir, normalizedNormal);

    vec3 ambientColor = dirLight.ambient * materialColor;
    vec3 diffuseColor = dirLight.diffuse * diffuseImpact * materialColor;
    vec3 specularColor = dirLight.specular * specularImpact * materialColor;

    return diffuseColor + specularColor + ambientColor;
}
