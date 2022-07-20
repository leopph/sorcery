//! #version 450 core

#include "TransformBuffer.glsl"

layout (location = 0) in vec3 in_Pos;
layout (location = 0) out vec3 out_TexCoords;


void main()
{
	gl_Position = (vec4(in_Pos, 1) * u_ViewProjMat).xyww;
	out_TexCoords = in_Pos;
}