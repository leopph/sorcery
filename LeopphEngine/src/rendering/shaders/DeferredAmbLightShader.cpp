#include "DeferredAmbLightShader.hpp"


namespace leopph::impl
{
	DeferredAmbLightShader::DeferredAmbLightShader() :
		Shader{s_LightPassVertSrc, s_AmbLightFragSrc}
	{}

}