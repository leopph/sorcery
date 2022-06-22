#version 410 core

layout (location = 0) out vec4 out_FragColor;

uniform sampler2D u_AccumTex;
uniform sampler2D u_RevealTex;


void main()
{
    vec4 accum = texelFetch(u_AccumTex, ivec2(gl_FragCoord.xy), 0);
    float reveal = texelFetch(u_RevealTex, ivec2(gl_FragCoord.xy), 0).r;
    out_FragColor = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), reveal);
}