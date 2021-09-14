#include "SkyboxShader.hpp"


namespace leopph::impl
{
	SkyboxShader::SkyboxShader() :
		Shader{s_SkyboxVertSrc, s_SkyboxFragSrc}
	{}

}