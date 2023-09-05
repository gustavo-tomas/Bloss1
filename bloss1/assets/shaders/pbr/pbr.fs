#version 460 core

// Don't forget to read this every day: https://learnopengl.com/PBR/Lighting

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

// Textures
struct Textures {

    // Material params
    sampler2D position;
    sampler2D normal;
    sampler2D albedo;
    sampler2D arm;
    sampler2D tbnNormal;
    sampler2D depth;

    // IBL
    samplerCube irradianceMap;
    samplerCube prefilterMap;
    sampler2D brdfLut;
};

uniform Textures textures;

// Lights
const int numberOfLights = 16;
struct Lights {

    // Point lights
    vec3 pointLightColors[numberOfLights];
    vec3 pointLightPositions[numberOfLights];

    // Directional lights
    vec3 dirLightDirections[numberOfLights];
    vec3 dirLightColors[numberOfLights];
};

uniform Lights lights;

uniform vec3 viewPos; // Camera position

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

void main() {

    // Retrieve data from gbuffer
    vec3 FragPos = texture(textures.position, fs_in.TexCoords).rgb;
    vec3 Normal = texture(textures.normal, fs_in.TexCoords).rgb;
    vec4 Albedo = texture(textures.albedo, fs_in.TexCoords);
    vec3 ARM = texture(textures.arm, fs_in.TexCoords).rgb;
    vec3 TBNNormal = texture(textures.tbnNormal, fs_in.TexCoords).rgb;
    float Depth = texture(textures.depth, fs_in.TexCoords).r;

    float AO = ARM.r;
    float Roughness = ARM.g;
    float Metalness = ARM.b;

    // Discard 'transparent' textures
    if (Albedo.a < 0.5) {
        discard;
    }

    // Normalized vectors
    vec3 N = TBNNormal;                       // Normalized normal
    vec3 V = normalize(viewPos - FragPos); // Normalized view direction
    vec3 R = reflect(-V, N);             // Normalized reflection

    // Metalness for a PBR workflow
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, Albedo.rgb, Metalness);

    // Reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < numberOfLights; i++) {

        // Calculate per-light radiance
        vec3 L = normalize(lights.pointLightPositions[i] - FragPos);
        vec3 H = normalize(V + L);

        float distance = length(lights.pointLightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance * distance);

        vec3 pointLightRadiance = lights.pointLightColors[i] * attenuation;
        vec3 dirLightRadiance = lights.dirLightColors[i] * max(dot(N, -lights.dirLightDirections[i]), 0.0) * Albedo.rgb;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, Roughness);
        float G = GeometrySmith(N, V, L, Roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // Fresnel
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - Metalness;

        // Add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);

        // Add to Lo
        Lo += (kD * Albedo.rgb / PI + specular) * pointLightRadiance * NdotL; // Point light influence
        Lo += dirLightRadiance;                                               // Dir light influence
    }

    // Irradiance (ambient light)
    vec3 irradiance = texture(textures.irradianceMap, N).rgb;

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - Metalness);
    vec3 diffuse = irradiance * Albedo.rgb;

    // Sample the pre-filter map and the BRDF lut and combine them together to get the IBL specular part
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(textures.prefilterMap, R, Roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(textures.brdfLut, vec2(max(dot(N, V), 0.0), Roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * AO;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    // Final color
    fragColor = vec4(color, 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
