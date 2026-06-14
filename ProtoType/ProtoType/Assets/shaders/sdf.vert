#version 300 es
precision highp float;

/**
 * \file
 * \author Junhwan Kim
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

layout(location = 0) in vec2 a_position;

layout (std140) uniform Camera
{
    mat3 u_view_projection;
};

uniform mat3 u_model_matrix;
uniform vec2 u_size; // padded quad size

out vec2 vTestPoint; // padded quad space

void main()
{
    vec3 ndc_point = u_view_projection * u_model_matrix * vec3(a_position, 1.0);
    gl_Position = vec4(ndc_point.xy, 0.0, 1.0);
    vTestPoint = a_position * u_size;
}