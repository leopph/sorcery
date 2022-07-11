#version 330 core

layout (location = 0) in vec2 in_Pos;


void main()
{
    gl_Position = vec4(in_Pos, 0, 1);
}