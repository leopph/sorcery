#include "ForObjShader.hpp"


namespace leopph::impl
{
	ForObjShader::ForObjShader() :
		ShaderProgram{GetStages()}
	{}


	std::vector<ShaderStage> ForObjShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_ObjectVertSrc, ShaderType::Vertex);
		ret.emplace_back(s_ObjectFragSrc, ShaderType::Fragment);
		return ret;
	}

}