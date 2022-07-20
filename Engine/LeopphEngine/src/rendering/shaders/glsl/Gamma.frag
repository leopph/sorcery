//! #version 450 core

layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) out vec4 out_FragColor;

uniform float u_GammaInverse;
layout(binding = 0) uniform sampler2D u_Image;


void main()
{
	vec3 originalColor = texelFetch(u_Image, ivec2(gl_FragCoord.xy), 0).rgb;
	out_FragColor = vec4(pow(originalColor.x, u_GammaInverse), pow(originalColor.y, u_GammaInverse), pow(originalColor.z, u_GammaInverse), 1);
}