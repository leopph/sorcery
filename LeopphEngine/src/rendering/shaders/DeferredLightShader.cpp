#include "DeferredLightShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DeferredLightShader::s_PosTexName{"u_PositionTexture"};
	const std::string DeferredLightShader::s_NormTexName{"u_NormalTexture"};
	const std::string DeferredLightShader::s_AmbTexName{"u_AmbientTexture"};
	const std::string DeferredLightShader::s_DiffTexName{"u_DiffuseTexture"};
	const std::string DeferredLightShader::s_SpecTexName{"u_SpecularTexture"};
	const std::string DeferredLightShader::s_ShineTexName{"u_ShineTexture"};


	DeferredLightShader::DeferredLightShader(const std::string_view vertSrc, const std::string_view fragSrc) :
		Shader{vertSrc, fragSrc},
		m_PosTexLoc{glGetUniformLocation(m_ProgramName, s_PosTexName.data())},
		m_NormTexLoc{glGetUniformLocation(m_ProgramName, s_NormTexName.data())},
		m_AmbTexLoc{glGetUniformLocation(m_ProgramName, s_AmbTexName.data())},
		m_DiffTexLoc{glGetUniformLocation(m_ProgramName, s_DiffTexName.data())},
		m_SpecTexLoc{glGetUniformLocation(m_ProgramName, s_SpecTexName.data())},
		m_ShineTexLoc{glGetUniformLocation(m_ProgramName, s_ShineTexName.data())}
	{}



	void DeferredLightShader::SetPositionTexture(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_PosTexLoc, unit);
	}


	void DeferredLightShader::SetNormalTexture(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_NormTexLoc, unit);
	}


	void DeferredLightShader::SetAmbientTexture(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_AmbTexLoc, unit);
	}


	void DeferredLightShader::SetDiffuseTexture(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_DiffTexLoc, unit);
	}


	void DeferredLightShader::SetSpecularTexture(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_SpecTexLoc, unit);
	}


	void DeferredLightShader::SetShineTexture(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_ShineTexLoc, unit);
	}
}