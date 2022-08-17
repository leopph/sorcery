//? #version 450 core

#ifndef CAMERA_DATA_GLSL
#define CAMERA_DATA_GLSL

layout(std140, binding = 1, row_major) uniform PerCameraData
{
	mat4 uViewMat;
	mat4 uViewMatInv;
	mat4 uProjMat;
	mat4 uProjMatInv;
	mat4 uViewProjMat;
	mat4 uViewProjMatInv;
	vec3 uCamPosition;
};


#endif