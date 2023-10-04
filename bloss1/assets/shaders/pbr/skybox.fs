#version 460 core

out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main() {
    vec3 envColor = texture(environmentMap, WorldPos).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2));

    // Posterization
    float levels = 8.0;
    float greyscale = max(envColor.r, max(envColor.g, envColor.b));

    float lower = floor(greyscale * levels) / levels;
    float lowerDiff = abs(greyscale - lower);

    float upper = ceil(greyscale * levels) / levels;
    float upperDiff = abs(upper - greyscale);

    float level = lowerDiff <= upperDiff ? lower : upper;
    float adjustment = level / greyscale;

    FragColor = vec4(envColor.rgb * adjustment, 1.0);

    // FragColor = vec4(envColor, 1.0);
}
