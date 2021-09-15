#include "DeferredAmbLightShader.hpp"


namespace leopph::impl
{
	DeferredAmbLightShader::DeferredAmbLightShader() :
		DeferredLightShader{s_LightPassVertSrc, s_AmbLightFragSrc}
	{}

}