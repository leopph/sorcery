//? #version 450 core

layout(std140, binding = 0) uniform TransformBuffer
{
	layout(row_major) mat4 u_ViewMat;
	layout(row_major) mat4 u_ViewMatInv;
	layout(row_major) mat4 u_ProjMat;
	layout(row_major) mat4 u_ProjMatInv;
	layout(row_major) mat4 u_ViewProjMat;
	layout(row_major) mat4 u_ViewProjMatInv;
};