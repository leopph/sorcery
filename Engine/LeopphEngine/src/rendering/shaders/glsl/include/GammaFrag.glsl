//? #version 450 core

#ifndef GAMMA_FRAG_GLSL
#define GAMMA_FRAG_GLSL

layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) out vec4 out_FragColor;

layout(binding = 0) uniform sampler2D u_Image;

const vec3 GAMMA_INVERSE = vec3(1 / 2.2);


void main()
{
	vec3 originalColor = texelFetch(u_Image, ivec2(gl_FragCoord.xy), 0).rgb;
	out_FragColor = vec4(pow(originalColor, GAMMA_INVERSE), 1);
}

#endif