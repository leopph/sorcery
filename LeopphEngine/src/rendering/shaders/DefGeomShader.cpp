#include "DefGeomShader.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DefGeomShader::s_ViewProjMatName{"u_ViewProjectionMatrix"};


	DefGeomShader::DefGeomShader() :
		ShaderProgram{GetStages()},
	m_ViewProjMatLoc{glGetUniformLocation(Name, s_ViewProjMatName.data())}
	{}


	void DefGeomShader::SetViewProjectionMatrix(const Matrix4& viewProjMat) const
	{
		glProgramUniformMatrix4fv(Name, m_ViewProjMatLoc, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(viewProjMat.Data().data()));
	}


	std::vector<ShaderStage> DefGeomShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_GPassObjectVertSrc, ShaderType::Vertex);
		ret.emplace_back(s_GPassObjectFragSrc, ShaderType::Fragment);
		return ret;
	}

}
