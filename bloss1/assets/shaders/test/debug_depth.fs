#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray depthMap;
uniform float near;
uniform float far;
uniform int layer;

// required when using a perspective projection matrix
float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    float depthValue = texture(depthMap, vec3(TexCoords, layer)).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}
