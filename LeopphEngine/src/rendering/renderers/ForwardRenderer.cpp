#include "ForwardRenderer.hpp"

#include "../../components/Camera.hpp"
#include "../../components/lighting/AmbientLight.hpp"
#include "../../components/lighting/DirLight.hpp"
#include "../../components/lighting/Light.hpp"
#include "../../components/lighting/PointLight.hpp"
#include "../../config/Settings.hpp"
#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"
#include "../../util/logger.h"
#include "../geometry/ModelImpl.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <string>
#include <utility>



namespace leopph::impl
{
	ForwardRenderer::ForwardRenderer() :
		m_ObjectShader{
			{
				{ShaderFamily::ObjectVertSrc, ShaderType::Vertex},
				{ShaderFamily::ObjectFragSrc, ShaderType::Fragment}
			}},
	m_SkyboxShader{
		{
			{ShaderFamily::SkyboxVertSrc, ShaderType::Vertex},
			{ShaderFamily::SkyboxFragSrc, ShaderType::Fragment}
		}},
	m_ShadowShader{
		{
			{ShaderFamily::ShadowMapVertSrc, ShaderType::Vertex}
		}}
	{
		glEnable(GL_DEPTH_TEST);

		Logger::Instance().Warning("The forward rendering pipeline is currently not feature complete. It is recommended to use the deferred pipeline.");
	}


	void ForwardRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active == nullptr)
		{
			return;
		}

		const auto camViewMat{Camera::Active->ViewMatrix()};
		const auto camProjMat{Camera::Active->ProjectionMatrix()};

		const auto& modelsAndMats{CalcAndCollectMatrices()};
		const auto& pointLights{CollectPointLights()};
		const auto& spotLights{CollectSpotLights()};

		RenderShadedObjects(camViewMat, camProjMat, modelsAndMats, pointLights, spotLights);
		RenderSkybox(camViewMat, camProjMat);
	}


	void ForwardRenderer::RenderShadedObjects(const Matrix4& camViewMat, 
											  const Matrix4& camProjMat,
											  const std::unordered_map<const ModelImpl*, std::vector< std::pair<Matrix4, Matrix4>>>& modelsAndMats,
											  const std::vector<const PointLight*>& pointLights,
											  const std::vector<const SpotLight*>& spotLights)
	{
		static auto objectFlagInfo{m_ObjectShader.GetFlagInfo()};
		auto& objectShader{m_ObjectShader.GetPermutation(objectFlagInfo)};

		objectShader.Use();

		objectShader.SetUniform("viewProjectionMatrix", camViewMat * camProjMat);
		objectShader.SetUniform("cameraPosition", Camera::Active->Entity.Transform->Position());

		/* Set up ambient light data */
		objectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (const auto dirLight = DataManager::DirectionalLight(); dirLight != nullptr)
		{
			objectShader.SetUniform("existsDirLight", true);
			objectShader.SetUniform("dirLight.direction", dirLight->Direction());
			objectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			objectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
		}
		else
		{
			objectShader.SetUniform("existsDirLight", false);
		}

		/* Set up PointLight data */
		objectShader.SetUniform("pointLightCount", static_cast<int>(pointLights.size()));
		for (std::size_t i = 0; i < pointLights.size(); i++)
		{
			const auto& pointLight = pointLights[i];

			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLight->Entity.Transform->Position());
			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
		}

		/* Set up SpotLight data */
		objectShader.SetUniform("spotLightCount", static_cast<int>(spotLights.size()));
		for (std::size_t i = 0; i < spotLights.size(); i++)
		{
			const auto& spotLight{spotLights[i]};

			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].position", spotLight->Entity.Transform->Position());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].direction", spotLight->Entity.Transform->Forward());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		/* Draw the shaded objects */
		for (const auto& [modelRes, matrices] : modelsAndMats)
		{
			modelRes->DrawShaded(objectShader, matrices, 0);
		}
	}


	void ForwardRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat)
	{
		static auto skyboxFlagInfo{m_SkyboxShader.GetFlagInfo()};
		auto& skyboxShader{m_SkyboxShader.GetPermutation(skyboxFlagInfo)};

		if (const auto& skybox{Camera::Active->Background().skybox}; skybox.has_value())
		{
			skyboxShader.Use();
			skyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			skyboxShader.SetUniform("projectionMatrix", camProjMat);
			DataManager::Skyboxes().find(skybox->AllFilePaths())->first.Draw(skyboxShader);
		}
	}
}
