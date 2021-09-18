#include "DefSpotShader.hpp"

#include "../../entity/Entity.hpp"
#include "../../math/LeopphMath.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DefSpotShader::s_ShadowMapName{"u_ShadowMap"};
	const std::string DefSpotShader::s_CamPosName{"u_CameraPosition"};
	const std::string DefSpotShader::s_LightPosName{"u_SpotLight.position"};
	const std::string DefSpotShader::s_LightDirName{"u_SpotLight.direction"};
	const std::string DefSpotShader::s_LightDiffName{"u_SpotLight.diffuseColor"};
	const std::string DefSpotShader::s_LightSpecName{"u_SpotLight.specularColor"};
	const std::string DefSpotShader::s_LightConstName{"u_SpotLight.constant"};
	const std::string DefSpotShader::s_LightLinName{"u_SpotLight.linear"};
	const std::string DefSpotShader::s_LightQuadName{"u_SpotLight.quadratic"};
	const std::string DefSpotShader::s_LightRangeName{"u_SpotLight.range"};
	const std::string DefSpotShader::s_LightInAngName{"u_SpotLight.innerAngleCosine"};
	const std::string DefSpotShader::s_LightOutAngName{"u_SpotLight.outerAngleCosine"};
	const std::string DefSpotShader::s_LightClipMatName{"u_LightWorldToClipMatrix"};


	DefSpotShader::DefSpotShader() :
		DefLightShader{GetStages()},
		m_ShadowMapLoc{glGetUniformLocation(Name, s_ShadowMapName.data())},
		m_CamPosLoc{glGetUniformLocation(Name, s_CamPosName.data())},
		m_LightPosLoc{glGetUniformLocation(Name, s_LightPosName.data())},
		m_LightDirLoc{glGetUniformLocation(Name, s_LightDirName.data())},
		m_LightDiffLoc(glGetUniformLocation(Name, s_LightDiffName.data())),
		m_LightSpecLoc{glGetUniformLocation(Name, s_LightSpecName.data())},
		m_LightConstLoc{glGetUniformLocation(Name, s_LightConstName.data())},
		m_LightLinLoc{glGetUniformLocation(Name, s_LightLinName.data())},
		m_LightQuadLoc{glGetUniformLocation(Name, s_LightQuadName.data())},
		m_LightRangeLoc{glGetUniformLocation(Name, s_LightRangeName.data())},
		m_LightInAngLoc{glGetUniformLocation(Name, s_LightInAngName.data())},
		m_LightOutAngLoc{glGetUniformLocation(Name, s_LightOutAngName.data())},
		m_LightClipMatLoc{glGetUniformLocation(Name, s_LightClipMatName.data())}
	{}


	void DefSpotShader::SetCameraPosition(const Vector3& pos) const
	{
		glProgramUniform3fv(Name, m_CamPosLoc, 1, pos.Data().data());
	}


	void DefSpotShader::SetSpotLight(const SpotLight& spotLight) const
	{
		glProgramUniform3fv(Name, m_LightPosLoc, 1, spotLight.entity.Transform->Position().Data().data());
		glProgramUniform3fv(Name, m_LightDirLoc, 1, spotLight.entity.Transform->Forward().Data().data());
		glProgramUniform3fv(Name, m_LightDiffLoc, 1, spotLight.Diffuse().Data().data());
		glProgramUniform3fv(Name, m_LightSpecLoc, 1, spotLight.Specular().Data().data());
		glProgramUniform1f(Name, m_LightConstLoc, spotLight.Constant());
		glProgramUniform1f(Name, m_LightLinLoc, spotLight.Linear());
		glProgramUniform1f(Name, m_LightQuadLoc, spotLight.Quadratic());
		glProgramUniform1f(Name, m_LightRangeLoc, spotLight.Range());
		glProgramUniform1f(Name, m_LightInAngLoc, math::Cos(math::ToRadians(spotLight.InnerAngle())));
		glProgramUniform1f(Name, m_LightOutAngLoc, math::Cos(math::ToRadians(spotLight.OuterAngle())));
	}


	void DefSpotShader::SetShadowMap(const int unit) const
	{
		glProgramUniform1i(Name, m_ShadowMapLoc, unit);
	}


	void DefSpotShader::SetLightClipMatrix(const Matrix4& mat) const
	{
		glProgramUniformMatrix4fv(Name, m_LightClipMatLoc, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(mat.Data().data()));
	}


	std::vector<ShaderStage> DefSpotShader::GetStages()
	{
		std::vector<ShaderStage> ret;
		ret.emplace_back(s_LightPassVertSrc, ShaderType::Vertex);
		ret.emplace_back(s_SpotLightPassFragSrc, ShaderType::Fragment);
		return ret;
	}

}