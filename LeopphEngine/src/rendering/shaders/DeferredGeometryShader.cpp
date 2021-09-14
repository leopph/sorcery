#include "DeferredGeometryShader.hpp"



namespace leopph::impl
{
	DeferredGeometryShader::DeferredGeometryShader() :
		Shader{s_GPassObjectVertSrc, s_GPassObjectFragSrc}
	{}

}
