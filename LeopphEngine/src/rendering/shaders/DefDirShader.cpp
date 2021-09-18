#include "DefDirShader.hpp"

#include "ShaderStage.hpp"

#include <glad/glad.h>



namespace leopph::impl
{
	const std::string DefDirShader::s_ShadowMapArrName{"u_ShadowMaps"};
	const std::string DefDirShader::s_CamPosName{"u_CameraPosition"};
	const std::string DefDirShader::s_DirLightDirName{"u_DirLight.direction"};
	const std::string DefDirShader::s_DirLightDiffName{"u_DirLight.diffuseColor"};
	const std::string DefDirShader::s_DirLightSpecName{"u_DirLight.specularColor"};
	const std::string DefDirShader::s_CascCountName{"u_CascadeCount"};
	const std::string DefDirShader::s_ClipMatName{"u_LightClipMatrices"};
	const std::string DefDirShader::s_CascBoundsName{"u_CascadeFarBounds"};


	DefDirShader::DefDirShader(const std::vector<std::string>& vertFlags, const std::vector<std::string>& fragFlags) :
		DefLightShader{GetStages(vertFlags, fragFlags)},
		m_ShadowMapArrLoc{glGetUniformLocation(Name, s_ShadowMapArrName.data())},
		m_CamPosLoc{glGetUniformLocation(Name, s_CamPosName.data())},
		m_DirLightDirLoc{glGetUniformLocation(Name, s_DirLightDirName.data())},
		m_DirLightDiffLoc{glGetUniformLocation(Name, s_DirLightDiffName.data())},
		m_DirLightSpecLoc{glGetUniformLocation(Name, s_DirLightSpecName.data())},
		m_CascCountLoc{glGetUniformLocation(Name, s_CascCountName.data())},
		m_ClipMatLoc{glGetUniformLocation(Name, s_ClipMatName.data())},
		m_CascBoundsLoc{glGetUniformLocation(Name, s_CascBoundsName.data())}
	{}


	void DefDirShader::SetShadowMaps(const std::vector<int>& shadowMaps) const
	{
		glProgramUniform1iv(Name, m_ShadowMapArrLoc, static_cast<GLsizei>(shadowMaps.size()), shadowMaps.data());
	}


	void DefDirShader::SetCameraPosition(const Vector3& pos) const
	{
		glProgramUniform3fv(Name, m_CamPosLoc, 1, pos.Data().data());
	}


	void DefDirShader::SetDirLight(const DirectionalLight& dirLight) const
	{
		glProgramUniform3fv(Name, m_DirLightDirLoc, 1, dirLight.Direction().Data().data());
		glProgramUniform3fv(Name, m_DirLightDiffLoc, 1, dirLight.Diffuse().Data().data());
		glProgramUniform3fv(Name, m_DirLightSpecLoc, 1, dirLight.Specular().Data().data());
	}


	void DefDirShader::SetCascadeCount(const unsigned count) const
	{
		glProgramUniform1ui(Name, m_CascCountLoc, count);
	}


	void DefDirShader::SetLightClipMatrices(const std::vector<Matrix4>& mats) const
	{
		glProgramUniformMatrix4fv(Name, m_ClipMatLoc, static_cast<GLsizei>(mats.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(mats.data()));
	}


	void DefDirShader::SetCascadeFarBounds(const std::vector<float>& bounds) const
	{
		glProgramUniform1fv(Name, m_CascBoundsLoc, static_cast<GLsizei>(bounds.size()), bounds.data());
	}


	std::vector<ShaderStage> DefDirShader::GetStages(const std::vector<std::string>& vertFlags, const std::vector<std::string>& fragFlags)
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_LightPassVertSrc, ShaderType::Vertex, vertFlags);
		ret.emplace_back(s_DirLightPassFragSrc, ShaderType::Fragment, fragFlags);
		return ret;
	}

}
