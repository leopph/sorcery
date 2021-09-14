#include "TextureShader.hpp"


namespace leopph::impl
{
	TextureShader::TextureShader() :
		Shader{s_LightPassVertSrc, s_TextureFragSrc}
	{}

}