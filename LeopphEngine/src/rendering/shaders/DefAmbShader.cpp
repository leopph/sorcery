#include "DefAmbShader.hpp"

#include "ShaderStage.hpp"

#include <glad/glad.h>

#include <vector>



namespace leopph::impl
{
	const std::string DefAmbShader::s_AmbLightName{"u_AmbientLight"};


	DefAmbShader::DefAmbShader() :
		DefLightShader{GetStages()},
		m_AmbLightLoc{glGetUniformLocation(Name, s_AmbLightName.data())}
	{}


	void DefAmbShader::SetAmbientLight(const Vector3& ambientLight) const
	{
		glProgramUniform3fv(Name, m_AmbLightLoc, 1, ambientLight.Data().data());
	}


	std::vector<ShaderStage> DefAmbShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_LightPassVertSrc, ShaderType::Vertex);
		ret.emplace_back(s_AmbLightFragSrc, ShaderType::Fragment);
		return ret;
	}

}
