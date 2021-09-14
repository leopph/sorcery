#include "DeferredDirLightShader.hpp"



namespace leopph::impl
{
	DeferredDirLightShader::DeferredDirLightShader() :
		Shader{s_LightPassVertSrc, s_DirLightPassFragSrc}
	{}

}
