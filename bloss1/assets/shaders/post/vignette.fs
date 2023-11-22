#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;
uniform float lens_radius;
uniform float lens_feathering;

void main() {
    fragColor = texture(screenTexture, fs_in.TexCoords);
    
	float dist = distance(fs_in.TexCoords, vec2(0.5,0.5));
	vec3 vig = vec3(smoothstep(lens_radius, (lens_radius - 0.001) * lens_feathering, dist));
	fragColor.xyz *= vig;
}
