#include "DeferredDirLightShader.hpp"

#include <glad/glad.h>



namespace leopph::impl
{
	const std::string DeferredDirLightShader::s_ShadowMapArrName{"u_ShadowMaps"};
	const std::string DeferredDirLightShader::s_CamPosName{"u_CameraPosition"};
	const std::string DeferredDirLightShader::s_DirLightDirName{"u_DirLight.direction"};
	const std::string DeferredDirLightShader::s_DirLightDiffName{"u_DirLight.diffuseColor"};
	const std::string DeferredDirLightShader::s_DirLightSpecName{"u_DirLight.specularColor"};
	const std::string DeferredDirLightShader::s_CascCountName{"u_CascadeCount"};
	const std::string DeferredDirLightShader::s_ClipMatName{"u_LightClipMatrices"};
	const std::string DeferredDirLightShader::s_CascBoundsName{"u_CascadeFarBounds"};


	DeferredDirLightShader::DeferredDirLightShader() :
		DeferredLightShader{s_LightPassVertSrc, s_DirLightPassFragSrc},
		m_ShadowMapArrLoc{glGetUniformLocation(m_ProgramName, s_ShadowMapArrName.data())},
		m_CamPosLoc{glGetUniformLocation(m_ProgramName, s_CamPosName.data())},
		m_DirLightDirLoc{glGetUniformLocation(m_ProgramName, s_DirLightDirName.data())},
		m_DirLightDiffLoc{glGetUniformLocation(m_ProgramName, s_DirLightDiffName.data())},
		m_DirLightSpecLoc{glGetUniformLocation(m_ProgramName, s_DirLightSpecName.data())},
		m_CascCountLoc{glGetUniformLocation(m_ProgramName, s_CascCountName.data())},
		m_ClipMatLoc{glGetUniformLocation(m_ProgramName, s_ClipMatName.data())},
		m_CascBoundsLoc{glGetUniformLocation(m_ProgramName, s_CascBoundsName.data())}
	{}


	void DeferredDirLightShader::SetShadowMaps(const std::vector<int>& shadowMaps) const
	{
		glProgramUniform1iv(m_ProgramName, m_ShadowMapArrLoc, static_cast<GLsizei>(shadowMaps.size()), shadowMaps.data());
	}


	void DeferredDirLightShader::SetCameraPosition(const Vector3& pos) const
	{
		glProgramUniform3fv(m_ProgramName, m_CamPosLoc, 1, pos.Data());
	}


	void DeferredDirLightShader::SetDirLight(const DirectionalLight& dirLight) const
	{
		glProgramUniform3fv(m_ProgramName, m_DirLightDirLoc, 1, dirLight.Direction().Data());
		glProgramUniform3fv(m_ProgramName, m_DirLightDiffLoc, 1, dirLight.Diffuse().Data());
		glProgramUniform3fv(m_ProgramName, m_DirLightSpecLoc, 1, dirLight.Specular().Data());
	}


	void DeferredDirLightShader::SetCascadeCount(const unsigned count) const
	{
		glProgramUniform1ui(m_ProgramName, m_CascCountLoc, count);
	}


	void DeferredDirLightShader::SetLightClipMatrices(const std::vector<Matrix4>& mats) const
	{
		glProgramUniformMatrix4fv(m_ProgramName, m_ClipMatLoc, static_cast<GLsizei>(mats.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(mats.data()));
	}


	void DeferredDirLightShader::SetCascadeFarBounds(const std::vector<float>& bounds) const
	{
		glProgramUniform1fv(m_ProgramName, m_CascBoundsLoc, static_cast<GLsizei>(bounds.size()), bounds.data());
	}

}
