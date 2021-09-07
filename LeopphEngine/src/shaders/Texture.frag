#version 460 core

layout (location = 0) in vec2 in_TexCoords;
out vec4 out_FragmentColor;

uniform sampler2D u_Texture;

void main()
{
    out_FragmentColor = vec4(vec3(texture(u_Texture, in_TexCoords).r), 1);
}