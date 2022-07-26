//? #version 450 core

#ifndef CAMERA_DATA_GLSL
#define CAMERA_DATA_GLSL


layout(binding = 0) uniform CameraBuffer
{
	layout(row_major) mat4 viewMat;
	layout(row_major) mat4 viewMatInv;
	layout(row_major) mat4 projMat;
	layout(row_major) mat4 projMatInv;
	layout(row_major) mat4 viewProjMat;
	layout(row_major) mat4 viewProjMatInv;
	vec3 position;
} u_CameraData;



mat4 GetCameraViewMatrix()
{
	return u_CameraData.viewMat;
}


mat4 GetCameraViewMatrixInverse()
{
	return u_CameraData.viewMatInv;
}


mat4 GetCameraProjectionMatrix()
{
	return u_CameraData.projMat;
}


mat4 GetCameraProjectionMatrixInverse()
{
	return u_CameraData.projMatInv;
}


mat4 GetCameraViewProjectionMatrix()
{
	return u_CameraData.viewProjMat;
}


mat4 GetCameraViewProjectionMatrixInverse()
{
	return u_CameraData.viewProjMatInv;
}


vec3 GetCameraPosition()
{
	return u_CameraData.position;
}

#endif