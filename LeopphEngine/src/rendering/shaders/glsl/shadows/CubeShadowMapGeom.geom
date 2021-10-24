#version 460 core

layout (triangles) in;

layout (triangle_strip, max_vertices = 18) out;

layout (location = 0) uniform mat4 u_ViewProjMats[6];

layout (location = 0) out vec4 out_FragPos;


void main()
{
	for (gl_Layer = 0; gl_Layer < 6; gl_Layer++)
	{
		for (int i = 0; i < 3; i++)
		{
			out_FragPos = gl_in[i].gl_Position;
			gl_Position = out_FragPos * u_ViewProjMats[gl_Layer];
			EmitVertex();
		}

		EndPrimitive();
	}
}