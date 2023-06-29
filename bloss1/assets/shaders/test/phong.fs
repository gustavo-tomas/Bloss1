#version 460 core

/**
* Simple phong shader mostly for debugging textures.
*/

layout (location = 0) out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
    vec3 FragPos;
    vec4 FragPosLightSpace;
} fs_in;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    sampler2D metalness;
    sampler2D roughness;
    sampler2D ao;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform PointLight pointLight;
uniform DirLight dirLight;

uniform vec3 viewPos;

vec3 calcDirLight(DirLight dirLight, vec3 normalizedNormal, vec3 normalizedViewDir);
vec3 calcPointLight(PointLight pointLight, vec3 normalizedNormal, vec3 normalizedViewDir);

float calculateSpecularImpact(vec3 normalizedLightDir, vec3 normalizedViewDir, vec3 normalizedNormal);
float calculateDiffuseImpact(vec3 normalizedNormal, vec3 normalizedLightDir);

// Material params
vec4 materialDiffuse = texture(material.diffuse, fs_in.TexCoords);
vec4 materialSpecular = texture(material.specular, fs_in.TexCoords);
vec4 materialNormal = texture(material.normal, fs_in.TexCoords);

void main() {
    // For transparent textures
    if (materialDiffuse.a < 0.5) {
        discard;
    }

    // Normalized vectors
    vec3 normalizedViewDir = normalize(viewPos - fs_in.FragPos);
    vec3 normalizedNormal = vec3(0.0);
    normalizedNormal = normalize(fs_in.TBN * (materialNormal.rgb * 2.0 - 1.0)); // range [0,1] -> [-1,1]
    // normalizedNormal = normalize(fs_in.Normal); // No normal map

	// Lights
    vec3 dirLightColor = calcDirLight(dirLight, normalizedNormal, normalizedViewDir);
    vec3 pointLightColor = calcPointLight(pointLight, normalizedNormal, normalizedViewDir);

	// Final color
    // vec3 finalColor = dirLightColor + pointLightColor;
    vec3 finalColor = dirLightColor;
    FragColor = vec4(finalColor, 1.0);
}

// ---------------------------------------------------------------------------------------------------------------------
vec3 calcDirLight(DirLight dirLight, vec3 normalizedNormal, vec3 normalizedViewDir) {
    vec3 normalizedLightDir = normalize(-dirLight.direction);
    // vec3 normalizedLightDir = normalize(dirLight.position - fs_in.FragPos);

	// Diffuse
    float diffuseImpact = calculateDiffuseImpact(normalizedNormal, normalizedLightDir);

	// Specular
    float specularImpact = calculateSpecularImpact(normalizedLightDir, normalizedViewDir, normalizedNormal);

    vec3 ambientColor = dirLight.ambient * materialDiffuse.rgb;
    vec3 diffuseColor = dirLight.diffuse * diffuseImpact * materialDiffuse.rgb;
    vec3 specularColor = dirLight.specular * specularImpact * materialSpecular.rgb;

    return diffuseColor + specularColor + ambientColor;
}

vec3 calcPointLight(PointLight pointLight, vec3 normalizedNormal, vec3 normalizedViewDir) {
    vec3 normalizedLightDir = normalize(pointLight.position - fs_in.FragPos);

	// Attenuation
    float distance = length(pointLight.position - fs_in.FragPos);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

	// Diffuse
    float diffuseImpact = calculateDiffuseImpact(normalizedNormal, normalizedLightDir);

	// Specular
    float specularImpact = calculateSpecularImpact(normalizedLightDir, normalizedViewDir, normalizedNormal);

    vec3 ambientColor = pointLight.ambient * materialDiffuse.rgb * attenuation;
    vec3 diffuseColor = pointLight.diffuse * diffuseImpact * materialDiffuse.rgb * attenuation;
    vec3 specularColor = pointLight.specular * specularImpact * materialSpecular.rgb * attenuation;

    return ambientColor + diffuseColor + specularColor;
}

float calculateSpecularImpact(vec3 normalizedLightDir, vec3 normalizedViewDir, vec3 normalizedNormal) {
    vec3 normalizedHalfwayDir = normalize(normalizedLightDir + normalizedViewDir);
    float specularImpact = pow(max(dot(normalizedNormal, normalizedHalfwayDir), 0.0), 32 * 2.0);

    return specularImpact;
}

float calculateDiffuseImpact(vec3 normalizedNormal, vec3 normalizedLightDir) {
    return max(dot(normalizedNormal, normalizedLightDir), 0.0);
}
