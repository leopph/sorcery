#version 460 core

layout (triangles) in;

layout (triangle_strip, max_vertices = 18) out;

layout (location = 0) uniform mat4 u_ViewProjMats[6];

layout (location = 0) out vec4 out_FragPos;


void EmitFace(mat4 viewProj)
{
	for (int i = 0; i < 3; i++)
	{
		out_FragPos = gl_in[i].gl_Position;
		gl_Position = out_FragPos * viewProj;
		EmitVertex();
	}

	EndPrimitive();
}


void main()
{
	gl_Layer = 0;
	gl_ViewportIndex = 0;
	EmitFace(u_ViewProjMats[0]);

	gl_Layer = 1;
	gl_ViewportIndex = 1;
	EmitFace(u_ViewProjMats[1]);

	gl_Layer = 2;
	gl_ViewportIndex = 2;
	EmitFace(u_ViewProjMats[2]);

	gl_Layer = 3;
	gl_ViewportIndex = 3;
	EmitFace(u_ViewProjMats[3]);

	gl_Layer = 4;
	gl_ViewportIndex = 4;
	EmitFace(u_ViewProjMats[4]);

	gl_Layer = 5;
	gl_ViewportIndex = 5;
	EmitFace(u_ViewProjMats[5]);
}