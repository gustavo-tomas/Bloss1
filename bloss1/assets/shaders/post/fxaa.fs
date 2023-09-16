#version 460 core

layout (location = 0) out vec4 fragColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

uniform sampler2D screenTexture;

void main() {
    vec2 texSize = textureSize(screenTexture, 0).xy;
    vec2 inverseScreenDimensions = 1.0 / texSize;
    vec3 rgbNW = texture(screenTexture, fs_in.TexCoords.xy + (vec2(-1.0, -1.0) * inverseScreenDimensions)).xyz;
    vec3 rgbNE = texture(screenTexture, fs_in.TexCoords.xy + (vec2(1.0, -1.0) * inverseScreenDimensions)).xyz;
    vec3 rgbSW = texture(screenTexture, fs_in.TexCoords.xy + (vec2(-1.0, 1.0) * inverseScreenDimensions)).xyz;
    vec3 rgbSE = texture(screenTexture, fs_in.TexCoords.xy + (vec2(1.0, 1.0) * inverseScreenDimensions)).xyz;
    vec3 rgbM = texture(screenTexture, fs_in.TexCoords.xy).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM = dot(rgbM, luma);

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * 0.25), 1e-6);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(8.0, 8.0), max(vec2(-8.0, -8.0), dir * rcpDirMin)) * inverseScreenDimensions;

    vec3 rgbA = 0.5 * (texture(screenTexture, fs_in.TexCoords.xy + dir * (1.0 / 3.0 - 0.5)).xyz + texture(screenTexture, fs_in.TexCoords.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(screenTexture, fs_in.TexCoords.xy + dir * -0.5).xyz + texture(screenTexture, fs_in.TexCoords.xy + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaM) && ((rgbB.x < rgbM.x) || (rgbB.y < rgbM.y) || (rgbB.z < rgbM.z))) {
        fragColor = vec4(rgbB, 1.0);
    } else {
        fragColor = vec4(rgbM, 1.0);
    }
}
