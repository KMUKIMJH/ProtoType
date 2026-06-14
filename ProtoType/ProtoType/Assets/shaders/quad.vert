#version 300 es

/**
 * \file
 * \author Junhwan Kim
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

layout(location = 2) in vec3 aModelRow0; // Row 0 of 3x3 model matrix
layout(location = 3) in vec3 aModelRow1; // Row 1 of 3x3 model matrix
layout(location = 4) in vec4 aTint;      // RGBA tint packed as a vec4 (0..1)
layout(location = 5) in vec2 aTexScale;  // (s_scale, t_scale)
layout(location = 6) in vec2 aTexOffset; // (s_offset, t_offset)
layout(location = 7) in int  aTextureIndex;

out vec2 vTexCoord;
flat out vec4 vTint;
flat out int  vTextureIndex;

layout(std140) uniform Camera
{
    mat3 u_view_projection;
};

void main()
{
    vec3 local_pos = vec3(aPos, 1.0);

    // Reconstruct model transform from two rows (row-major), last row is (0,0,1)
    mat3 model = mat3(
        aModelRow0.x, aModelRow1.x, 0.0,
        aModelRow0.y, aModelRow1.y, 0.0,
        aModelRow0.z, aModelRow1.z, 1.0
    );

    vec3 ndc_point = u_view_projection * (model * local_pos);
    gl_Position = vec4(ndc_point.xy, 0.0, 1.0);

    vTexCoord     = aTexCoord * aTexScale + aTexOffset;
    vTint         = aTint;
    vTextureIndex = aTextureIndex;
}
