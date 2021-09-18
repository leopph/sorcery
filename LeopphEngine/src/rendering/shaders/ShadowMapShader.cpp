#include "ShadowMapShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string ShadowMapShader::s_WorldToClipMatName{"u_LightWorldToClipMatrix"};


	ShadowMapShader::ShadowMapShader() :
		ShaderProgram{GetStages()},
		m_WorldToClipMatLoc{glGetUniformLocation(Name, s_WorldToClipMatName.data())}
	{}


	void ShadowMapShader::SetLightWorldToClipMatrix(const Matrix4& mat) const
	{
		glProgramUniformMatrix4fv(Name, m_WorldToClipMatLoc, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(mat.Data().data()));
	}


	std::vector<ShaderStage> ShadowMapShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_ShadowMapVertSrc, ShaderType::Vertex);
		return ret;
	}

}