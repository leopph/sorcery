#include "ShadowMapShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string ShadowMapShader::s_WorldToClipMatName{"u_LightWorldToClipMatrix"};


	ShadowMapShader::ShadowMapShader() :
		Shader{s_DirShadowMapVertSrc, {}},
		m_WorldToClipMatLoc{glGetUniformLocation(m_ProgramName, s_WorldToClipMatName.data())}
	{}


	void ShadowMapShader::SetLightWorldToClipMatrix(const Matrix4& mat) const
	{
		glProgramUniformMatrix4fv(m_ProgramName, m_WorldToClipMatLoc, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(mat.Data().data()));
	}

}