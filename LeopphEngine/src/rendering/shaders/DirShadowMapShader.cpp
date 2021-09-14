#include "DirShadowMapShader.hpp"


namespace leopph::impl
{
	DirShadowMapShader::DirShadowMapShader() :
		Shader{s_DirShadowMapVertSrc, s_DirShadowMapFragSrc}
	{}

}