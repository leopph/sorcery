#version 420 core


//! #define DIRLIGHT
//! #define NUM_SPOT 3
//! #define NUM_POINT 3


#ifdef DIRLIGHT
struct DirLight
{
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
};
#endif


#if NUM_SPOT > 0
struct SpotLight
{
	vec3 position;
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
	float range;
	float innerCos;
	float outerCos;
};
#endif


#if NUM_POINT > 0
struct PointLight
{
	vec3 position;
	vec3 diffuseColor;
	vec3 specularColor;
	float range;
};
#endif



layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in mat4 in_ModelMat;
layout (location = 7) in mat4 in_NormalMat;


layout (location = 0) out vec3 out_FragPos;
layout (location = 1) out vec3 out_Normal;
layout (location = 2) out vec2 out_TexCoords;


layout(std140, binding = 0) uniform PerFrameConstants
{
	layout(row_major) mat4 u_ViewProjMat;
	vec3 u_AmbLight;
	vec3 u_CamPos;

	#ifdef DIRLIGHT
	DirLight u_DirLight;
	#endif

	#if NUM_SPOT > 0
	SpotLight u_SpotLights[NUM_SPOT];
	#endif

	#if NUM_POINT > 0
	PointLight u_PointLights[NUM_POINT];
	#endif
};


void main()
{
    vec4 fragPosWorldSpace = vec4(in_Pos, 1.0) * in_ModelMat;
    out_Normal = in_Normal * mat3(in_NormalMat);
    out_FragPos = fragPosWorldSpace.xyz;
    out_TexCoords = in_TexCoords;
    gl_Position = fragPosWorldSpace * u_ViewProjMat;
}