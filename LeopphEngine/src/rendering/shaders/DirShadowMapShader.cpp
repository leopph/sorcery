#include "DirShadowMapShader.hpp"

#include <string_view>


namespace leopph::impl
{
	DirShadowMapShader::DirShadowMapShader() :
		Shader{s_DirShadowMapVertSrc, std::string_view{}}
	{}

}