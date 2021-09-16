#include "DeferredGeometryShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DeferredGeometryShader::s_ViewProjMatName{"u_ViewProjectionMatrix"};


	DeferredGeometryShader::DeferredGeometryShader() :
		Shader{s_GPassObjectVertSrc, s_GPassObjectFragSrc},
	m_ViewProjMatLoc{glGetUniformLocation(m_ProgramName, s_ViewProjMatName.data())}
	{}


	void DeferredGeometryShader::SetViewProjectionMatrix(const Matrix4& viewProjMat) const
	{
		glProgramUniformMatrix4fv(m_ProgramName, m_ViewProjMatLoc, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(viewProjMat.Data().data()));
	}
}
