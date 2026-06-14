#version 300 es
precision highp float;

/**
 * \file
 * \author Junhwan Kim
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

in vec2 vTestPoint;
layout(location = 0) out vec4 FragColor;

uniform vec4 u_fillColor;
uniform vec4 u_lineColor;
uniform vec2 u_shapeSize;
uniform vec2 u_size; // padded quad size
uniform float u_lineWidth;
uniform int u_shapeType;

float sdCircle(vec2 p, float r){ return length(p) - r; }
float sdRectangle(vec2 p, vec2 half_dim)
{
	vec2 d = abs(p) - half_dim;
	float outside = length(max(d, vec2(0.0)));
	float inside = min(max(d.x, d.y),0.0);
	return outside + inside;
}

void main()
{
	// convert from padded-quad space to shape space
	vec2 scale = u_shapeSize / max(u_size, vec2(0.0001));
	vec2 p = vTestPoint * scale;

	float sdf =0.0;
	if (u_shapeType ==0)
	{
		float r =0.5 * min(u_shapeSize.x, u_shapeSize.y);
		sdf = sdCircle(p, r);
	}
	else if (u_shapeType ==1)
	{
		sdf = sdRectangle(p,0.5 * u_shapeSize);
	}

	float halfW =0.5 * u_lineWidth;
	float distToBand = abs(sdf) - halfW;
	// Smaller AA to avoid visible growth. Ensure at least ~1px AA.
	float w = max(0.75, fwidth(sdf));
	float alphaOutline =1.0 - smoothstep(-w, w, distToBand);

	float alphaFill = sdf <0.0 ?1.0 :0.0;
	vec4 fillColor = vec4(u_fillColor.rgb, alphaFill * u_fillColor.a);
	vec4 lineColor = vec4(u_lineColor.rgb, alphaOutline * u_lineColor.a);

	vec4 outColor = lineColor.a >0.0 ? lineColor : fillColor;
	if (outColor.a <=0.0) discard;
	FragColor = outColor;
}