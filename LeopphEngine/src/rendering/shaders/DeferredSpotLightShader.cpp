#include "DeferredSpotLightShader.hpp"

#include "../../entity/Entity.hpp"
#include "../../math/LeopphMath.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	const std::string DeferredSpotLightShader::s_ShadowMapName{"u_ShadowMap"};
	const std::string DeferredSpotLightShader::s_CamPosName{"u_CameraPosition"};
	const std::string DeferredSpotLightShader::s_LightPosName{"u_SpotLight.position"};
	const std::string DeferredSpotLightShader::s_LightDirName{"u_SpotLight.direction"};
	const std::string DeferredSpotLightShader::s_LightDiffName{"u_SpotLight.diffuseColor"};
	const std::string DeferredSpotLightShader::s_LightSpecName{"u_SpotLight.specularColor"};
	const std::string DeferredSpotLightShader::s_LightConstName{"u_SpotLight.constant"};
	const std::string DeferredSpotLightShader::s_LightLinName{"u_SpotLight.linear"};
	const std::string DeferredSpotLightShader::s_LightQuadName{"u_SpotLight.quadratic"};
	const std::string DeferredSpotLightShader::s_LightRangeName{"u_SpotLight.range"};
	const std::string DeferredSpotLightShader::s_LightInAngName{"u_SpotLight.innerAngleCosine"};
	const std::string DeferredSpotLightShader::s_LightOutAngName{"u_SpotLight.outerAngleCosine"};
	const std::string DeferredSpotLightShader::s_LightClipMatName{"u_LightClipMatrix"};


	DeferredSpotLightShader::DeferredSpotLightShader() :
		DeferredLightShader{s_LightPassVertSrc, s_SpotLightPassFragSrc},
		m_ShadowMapLoc{glGetUniformLocation(m_ProgramName, s_ShadowMapName.data())},
		m_CamPosLoc{glGetUniformLocation(m_ProgramName, s_CamPosName.data())},
		m_LightPosLoc{glGetUniformLocation(m_ProgramName, s_LightPosName.data())},
		m_LightDirLoc{glGetUniformLocation(m_ProgramName, s_LightDirName.data())},
		m_LightDiffLoc(glGetUniformLocation(m_ProgramName, s_LightDiffName.data())),
		m_LightSpecLoc{glGetUniformLocation(m_ProgramName, s_LightSpecName.data())},
		m_LightConstLoc{glGetUniformLocation(m_ProgramName, s_LightConstName.data())},
		m_LightLinLoc{glGetUniformLocation(m_ProgramName, s_LightLinName.data())},
		m_LightQuadLoc{glGetUniformLocation(m_ProgramName, s_LightQuadName.data())},
		m_LightRangeLoc{glGetUniformLocation(m_ProgramName, s_LightRangeName.data())},
		m_LightInAngLoc{glGetUniformLocation(m_ProgramName, s_LightInAngName.data())},
		m_LightOutAngLoc{glGetUniformLocation(m_ProgramName, s_LightOutAngName.data())},
		m_LightClipMatLoc{glGetUniformLocation(m_ProgramName, s_LightClipMatName.data())}
	{}


	void DeferredSpotLightShader::SetCameraPosition(const Vector3& pos) const
	{
		glProgramUniform3fv(m_ProgramName, m_CamPosLoc, 1, pos.Data().data());
	}


	void DeferredSpotLightShader::SetSpotLight(const SpotLight& spotLight) const
	{
		glProgramUniform3fv(m_ProgramName, m_LightPosLoc, 1, spotLight.entity.Transform->Position().Data().data());
		glProgramUniform3fv(m_ProgramName, m_LightDirLoc, 1, spotLight.entity.Transform->Forward().Data().data());
		glProgramUniform3fv(m_ProgramName, m_LightDiffLoc, 1, spotLight.Diffuse().Data().data());
		glProgramUniform3fv(m_ProgramName, m_LightSpecLoc, 1, spotLight.Specular().Data().data());
		glProgramUniform1f(m_ProgramName, m_LightConstLoc, spotLight.Constant());
		glProgramUniform1f(m_ProgramName, m_LightLinLoc, spotLight.Linear());
		glProgramUniform1f(m_ProgramName, m_LightQuadLoc, spotLight.Quadratic());
		glProgramUniform1f(m_ProgramName, m_LightRangeLoc, spotLight.Range());
		glProgramUniform1f(m_ProgramName, m_LightInAngLoc, math::Cos(math::ToRadians(spotLight.InnerAngle())));
		glProgramUniform1f(m_ProgramName, m_LightOutAngLoc, math::Cos(math::ToRadians(spotLight.OuterAngle())));
	}


	void DeferredSpotLightShader::SetShadowMap(const int unit) const
	{
		glProgramUniform1i(m_ProgramName, m_ShadowMapLoc, unit);
	}


	void DeferredSpotLightShader::SetLightClipMatrix(const Matrix4& mat) const
	{
		glProgramUniformMatrix4fv(m_ProgramName, m_LightClipMatLoc, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(mat.Data().data()));
	}

}