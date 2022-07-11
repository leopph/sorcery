#version 450 core

layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) in vec2 in_TexCoords;
layout(location = 0) out vec4 out_FragColor;

uniform float u_GammaInverse;
uniform sampler2D u_Image;


void main()
{
	//vec3 originalColor = texelFetch(u_Image, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 originalColor = texture(u_Image, in_TexCoords).rgb;
	out_FragColor = vec4(pow(originalColor.x, u_GammaInverse), pow(originalColor.y, u_GammaInverse), pow(originalColor.z, u_GammaInverse), 1);
}