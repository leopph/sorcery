#include "DeferredDirLightShader.hpp"

#include <glad/glad.h>



namespace leopph::impl
{
	const std::string DeferredDirLightShader::s_PosTexName{"u_PositionTexture"};
	const std::string DeferredDirLightShader::s_NormTexName{"u_NormalTexture"};
	const std::string DeferredDirLightShader::s_DiffTexName{"u_DiffuseTexture"};
	const std::string DeferredDirLightShader::s_SpecTexName{"u_SpecularTexture"};
	const std::string DeferredDirLightShader::s_ShineTexName{"u_ShineTexture"};
	const std::string DeferredDirLightShader::s_ShadowMapArrName{"u_ShadowMaps"};
	const std::string DeferredDirLightShader::s_CamPosName{"u_CameraPosition"};
	const std::string DeferredDirLightShader::s_DirLightDirName{"u_DirLight.direction"};
	const std::string DeferredDirLightShader::s_DirLightDiffName{"u_DirLight.diffuseColor"};
	const std::string DeferredDirLightShader::s_DirLightSpecName{"u_DirLight.specularColor"};
	const std::string DeferredDirLightShader::s_CascCountName{"u_CascadeCount"};
	const std::string DeferredDirLightShader::s_ClipMatName{"u_LightClipMatrices"};
	const std::string DeferredDirLightShader::s_CascBoundsName{"u_CascadeFarBounds"};


	DeferredDirLightShader::DeferredDirLightShader() :
		Shader{s_LightPassVertSrc, s_DirLightPassFragSrc},
	m_PosTexLoc{glGetUniformLocation(Id, s_PosTexName.data())},
	m_NormTexLoc{glGetUniformLocation(Id, s_NormTexName.data())},
	m_DiffTexLoc{glGetUniformLocation(Id, s_DiffTexName.data())},
	m_SpecTexLoc{glGetUniformLocation(Id, s_SpecTexName.data())},
	m_ShineTexLoc{glGetUniformLocation(Id, s_ShineTexName.data())},
	m_ShadowMapArrLoc{glGetUniformLocation(Id, s_ShadowMapArrName.data())},
	m_CamPosLoc{glGetUniformLocation(Id, s_CamPosName.data())},
	m_DirLightDirLoc{glGetUniformLocation(Id, s_DirLightDirName.data())},
	m_DirLightDiffLoc{glGetUniformLocation(Id, s_DirLightDiffName.data())},
	m_DirLightSpecLoc{glGetUniformLocation(Id, s_DirLightSpecName.data())},
	m_CascCountLoc{glGetUniformLocation(Id, s_CascCountName.data())},
	m_ClipMatLoc{glGetUniformLocation(Id, s_ClipMatName.data())},
	m_CascBoundsLoc{glGetUniformLocation(Id, s_CascBoundsName.data())}
	{}


	void DeferredDirLightShader::SetPositionTexture(const int unit) const
	{
		glProgramUniform1i(Id, m_PosTexLoc, unit);
	}


	void DeferredDirLightShader::SetNormalTexture(const int unit) const
	{
		glProgramUniform1i(Id, m_NormTexLoc, unit);
	}


	void DeferredDirLightShader::SetDiffuseTexture(const int unit) const
	{
		glProgramUniform1i(Id, m_DiffTexLoc, unit);
	}


	void DeferredDirLightShader::SetSpecularTexture(const int unit) const
	{
		glProgramUniform1i(Id, m_SpecTexLoc, unit);
	}


	void DeferredDirLightShader::SetShineTexture(const int unit) const
	{
		glProgramUniform1i(Id, m_ShineTexLoc, unit);
	}


	void DeferredDirLightShader::SetShadowMaps(const std::vector<int>& shadowMaps) const
	{
		glProgramUniform1iv(Id, m_ShadowMapArrLoc, static_cast<GLsizei>(shadowMaps.size()), shadowMaps.data());
	}


	void DeferredDirLightShader::SetCameraPosition(const Vector3& pos) const
	{
		glProgramUniform3fv(Id, m_CamPosLoc, 1, pos.Data());
	}


	void DeferredDirLightShader::SetDirLight(const DirectionalLight& dirLight) const
	{
		glProgramUniform3fv(Id, m_DirLightDirLoc, 1, dirLight.Direction().Data());
		glProgramUniform3fv(Id, m_DirLightDiffLoc, 1, dirLight.Diffuse().Data());
		glProgramUniform3fv(Id, m_DirLightSpecLoc, 1, dirLight.Specular().Data());
	}


	void DeferredDirLightShader::SetCascadeCount(const unsigned count) const
	{
		glProgramUniform1ui(Id, m_CascCountLoc, count);
	}


	void DeferredDirLightShader::SetLightClipMatrices(const std::vector<Matrix4>& mats) const
	{
		glProgramUniformMatrix4fv(Id, m_ClipMatLoc, static_cast<GLsizei>(mats.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(mats.data()));
	}


	void DeferredDirLightShader::SetCascadeFarBounds(const std::vector<float>& bounds) const
	{
		glProgramUniform1fv(Id, m_CascBoundsLoc, static_cast<GLsizei>(bounds.size()), bounds.data());
	}

}
