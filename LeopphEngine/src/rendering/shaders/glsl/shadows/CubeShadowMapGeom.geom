#version 460 core

layout (triangles) in;

layout (triangle_strip, max_vertices = 18) out;

layout (location = 0) uniform mat4 u_ViewProjMats[6];

layout (location = 0) out vec4 out_FragPos;


void main()
{
	for (int i = 0; i < 6; i++)
	{
		gl_Layer = i;

		for (int j = 0; j < 3; j++)
		{
			out_FragPos = gl_in[j].gl_Position;
			gl_Position = out_FragPos * u_ViewProjMats[i];
			EmitVertex();
		}

		EndPrimitive();
	}
}