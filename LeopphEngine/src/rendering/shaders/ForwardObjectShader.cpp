#include "ForwardObjectShader.hpp"


namespace leopph::impl
{
	ForwardObjectShader::ForwardObjectShader() :
		Shader{s_ObjectVertSrc, s_ObjectFragSrc}
	{}

}