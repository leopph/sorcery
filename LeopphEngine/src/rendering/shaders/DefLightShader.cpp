#include "DefLightShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DefLightShader::s_PosTexName{"u_PositionTexture"};
	const std::string DefLightShader::s_NormTexName{"u_NormalTexture"};
	const std::string DefLightShader::s_AmbTexName{"u_AmbientTexture"};
	const std::string DefLightShader::s_DiffTexName{"u_DiffuseTexture"};
	const std::string DefLightShader::s_SpecTexName{"u_SpecularTexture"};
	const std::string DefLightShader::s_ShineTexName{"u_ShineTexture"};


	DefLightShader::DefLightShader(const std::vector<ShaderStage>& stages) :
		ShaderProgram{stages},
		m_PosTexLoc{glGetUniformLocation(Name, s_PosTexName.data())},
		m_NormTexLoc{glGetUniformLocation(Name, s_NormTexName.data())},
		m_AmbTexLoc{glGetUniformLocation(Name, s_AmbTexName.data())},
		m_DiffTexLoc{glGetUniformLocation(Name, s_DiffTexName.data())},
		m_SpecTexLoc{glGetUniformLocation(Name, s_SpecTexName.data())},
		m_ShineTexLoc{glGetUniformLocation(Name, s_ShineTexName.data())}
	{}



	void DefLightShader::SetPositionTexture(const int unit) const
	{
		glProgramUniform1i(Name, m_PosTexLoc, unit);
	}


	void DefLightShader::SetNormalTexture(const int unit) const
	{
		glProgramUniform1i(Name, m_NormTexLoc, unit);
	}


	void DefLightShader::SetAmbientTexture(const int unit) const
	{
		glProgramUniform1i(Name, m_AmbTexLoc, unit);
	}


	void DefLightShader::SetDiffuseTexture(const int unit) const
	{
		glProgramUniform1i(Name, m_DiffTexLoc, unit);
	}


	void DefLightShader::SetSpecularTexture(const int unit) const
	{
		glProgramUniform1i(Name, m_SpecTexLoc, unit);
	}


	void DefLightShader::SetShineTexture(const int unit) const
	{
		glProgramUniform1i(Name, m_ShineTexLoc, unit);
	}
}