#include "TextureShader.hpp"


namespace leopph::impl
{
	TextureShader::TextureShader() :
		ShaderProgram{GetStages()}
	{}


	std::vector<ShaderStage> TextureShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_LightPassVertSrc, ShaderType::Vertex);
		ret.emplace_back(s_TextureFragSrc, ShaderType::Fragment);
		return ret;
	}

}