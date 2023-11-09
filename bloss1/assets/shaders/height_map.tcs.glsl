#version 460 core

layout (vertices = 4) out;

in vec2 TexCoord[];

out vec2 TextureCoord[];

uniform mat4 model;
uniform mat4 view;

uniform int min_tess_level = 4;
uniform int max_tess_level = 64;
uniform float min_distance = 20;
uniform float max_distance = 800;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    TextureCoord[gl_InvocationID] = TexCoord[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        // Step 1: transform each vertex into eye space
        // ---------------------------------------------------------------------
        vec4 eyeSpacePos00 = view * model * gl_in[0].gl_Position;
        vec4 eyeSpacePos01 = view * model * gl_in[1].gl_Position;
        vec4 eyeSpacePos10 = view * model * gl_in[2].gl_Position;
        vec4 eyeSpacePos11 = view * model * gl_in[3].gl_Position;

        // Step 2: "distance" from camera scaled between 0 and 1
        // ---------------------------------------------------------------------
        float distance00 = clamp((abs(eyeSpacePos00.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        float distance01 = clamp((abs(eyeSpacePos01.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        float distance10 = clamp((abs(eyeSpacePos10.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        float distance11 = clamp((abs(eyeSpacePos11.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);

        // Step 3: interpolate edge tessellation level based on closer vertex
        // ---------------------------------------------------------------------
        float tessLevel0 = mix(max_tess_level, min_tess_level, min(distance10, distance00));
        float tessLevel1 = mix(max_tess_level, min_tess_level, min(distance00, distance01));
        float tessLevel2 = mix(max_tess_level, min_tess_level, min(distance01, distance11));
        float tessLevel3 = mix(max_tess_level, min_tess_level, min(distance11, distance10));

        // Step 4: set the corresponding outer edge tessellation levels
        // ---------------------------------------------------------------------
        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        // ----------------------------------------------------------------------
        // Step 5: set the inner tessellation levels to the max of the two parallel edges
        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}
