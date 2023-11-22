#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

// Textures
uniform sampler2D screenTexture;
uniform vec3 edge_color;
uniform float threshold;

vec4[9] make_kernel(vec2 size)
{
    vec4 n[9];
	float w = 1.0 / size.x;
	float h = 1.0 / size.y;

	n[0] = texture(screenTexture, fs_in.TexCoords + vec2( -w, -h));
	n[1] = texture(screenTexture, fs_in.TexCoords + vec2(0.0, -h));
	n[2] = texture(screenTexture, fs_in.TexCoords + vec2(  w, -h));
	n[3] = texture(screenTexture, fs_in.TexCoords + vec2( -w, 0.0));
	n[4] = texture(screenTexture, fs_in.TexCoords);
	n[5] = texture(screenTexture, fs_in.TexCoords + vec2(  w, 0.0));
	n[6] = texture(screenTexture, fs_in.TexCoords + vec2( -w, h));
	n[7] = texture(screenTexture, fs_in.TexCoords + vec2(0.0, h));
	n[8] = texture(screenTexture, fs_in.TexCoords + vec2(  w, h));

    return n;
}

void main() {

    vec4 color = texture(screenTexture, fs_in.TexCoords);

    vec2 texSize = textureSize(screenTexture, 0).xy;
    vec4 n[9] = make_kernel(texSize.xy);

    vec4 sobel_edge_h = n[2] + (2.0 * n[5]) + n[8] - (n[0] + (2.0 * n[3]) + n[6]);
  	vec4 sobel_edge_v = n[0] + (2.0 * n[1]) + n[2] - (n[6] + (2.0 * n[7]) + n[8]);
	vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

    if (length(sobel.rgb) > threshold)
    {
	    fragColor = vec4(edge_color, color.a);
    }

    else
    {
	    fragColor = color;
    }

	// fragColor = vec4(1.0 - sobel.rgb, 1.0);
}
