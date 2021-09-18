#include "SkyboxShader.hpp"


namespace leopph::impl
{
	SkyboxShader::SkyboxShader() :
		ShaderProgram{GetStages()}
	{}



	std::vector<ShaderStage> SkyboxShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_SkyboxVertSrc, ShaderType::Vertex);
		ret.emplace_back(s_SkyboxFragSrc, ShaderType::Fragment);
		return ret;
	}

}