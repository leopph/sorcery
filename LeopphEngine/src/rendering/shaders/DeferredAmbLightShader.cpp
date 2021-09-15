#include "DeferredAmbLightShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DeferredAmbLightShader::s_AmbLightName{"u_AmbientLight"};


	DeferredAmbLightShader::DeferredAmbLightShader() :
		DeferredLightShader{s_LightPassVertSrc, s_AmbLightFragSrc},
		m_AmbLightLoc{glGetUniformLocation(m_ProgramName, s_AmbLightName.data())}
	{}


	void DeferredAmbLightShader::SetAmbientLight(const Vector3& ambientLight) const
	{
		glProgramUniform3fv(m_ProgramName, m_AmbLightLoc, 1, ambientLight.Data());
	}

}