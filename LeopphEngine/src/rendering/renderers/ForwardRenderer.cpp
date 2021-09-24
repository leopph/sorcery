#include "ForwardRenderer.hpp"

#include "../SkyboxResource.hpp"
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
#include "../geometry/ModelResource.hpp"

#include <glad/glad.h>

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
	}


	void ForwardRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
		{
			return;
		}

		const auto camViewMat{Camera::Active()->ViewMatrix()};
		const auto camProjMat{Camera::Active()->ProjectionMatrix()};

		const auto& modelsAndMats{CalcAndCollectMatrices()};
		const auto& pointLights{CollectPointLights()};
		const auto& spotLights{CollectSpotLights()};

		const auto& lightTransformMat{RenderDirectionalShadowMap(modelsAndMats)};
		//RenderPointShadowMaps();
		RenderShadedObjects(camViewMat, camProjMat, lightTransformMat, modelsAndMats, pointLights, spotLights);
		RenderSkybox(camViewMat, camProjMat);
	}


	std::optional<Matrix4> ForwardRenderer::RenderDirectionalShadowMap(const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		const auto& dirLight = DataManager::DirectionalLight();

		if (dirLight == nullptr)
		{
			return {};
		}

		static auto shadowFlagInfo{m_ShadowShader.GetFlagInfo()};
		auto& shadowShader{m_ShadowShader.GetPermutation(shadowFlagInfo)};

		const auto& shadowMaps{DataManager::ShadowMaps()};

		/* If we don't have a shadow map, we create one */
		if (shadowMaps.empty())
		{
			DataManager::CreateShadowMap(Settings::DirectionalShadowMapResolutions()[0]); // TODO cascades
		}

		const auto lightViewMat{Matrix4::LookAt(-dirLight->Direction(), Vector3{}, Vector3::Up())};
		const auto lightProjMat{Matrix4::Ortographic(-10, 10, 10, -10, Camera::Active()->NearClipPlane(), Camera::Active()->FarClipPlane())};
		const auto lightTransformMat{lightViewMat * lightProjMat};

		shadowShader.Use();
		shadowMaps.front().BindForWriting();

		shadowShader.SetUniform("u_LightWorldToClipMatrix", lightTransformMat);

		for (const auto& [modelRes, matrices] : modelsAndMats)
		{
			modelRes->DrawDepth(matrices.first);
		}

		shadowMaps.front().UnbindFromWriting();

		return {lightTransformMat};
	}


	void ForwardRenderer::RenderPointShadowMaps(const std::vector<const PointLight*>& pointLights,
												const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats)
	{
		const auto& shadowMaps{DataManager::ShadowMaps()};

		/* If we lack the necessary number of shadow maps, we create new ones */
		while (pointLights.size() > shadowMaps.size())
		{
			DataManager::CreateShadowMap(Settings::PointLightShadowMapResolution());
		}

		/* Iterate over the lights and use a different shadow map for each */
		for (auto shadowMapIt{DataManager::DirectionalLight() == nullptr ? shadowMaps.begin() : ++shadowMaps.begin()};
		     const auto& pointLight : pointLights)
		{
			for (const auto& [modelRes, matrices] : modelsAndMats)
			{
				modelRes->DrawDepth(matrices.first);
			}

			++shadowMapIt;
		}
	}


	void ForwardRenderer::RenderShadedObjects(const Matrix4& camViewMat, 
											  const Matrix4& camProjMat,
											  const std::optional<Matrix4>& lightTransformMat,
											  const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats,
											  const std::vector<const PointLight*>& pointLights,
											  const std::vector<const SpotLight*>& spotLights)
	{
		static auto objectFlagInfo{m_ObjectShader.GetFlagInfo()};
		auto& objectShader{m_ObjectShader.GetPermutation(objectFlagInfo)};

		objectShader.Use();

		objectShader.SetUniform("viewProjectionMatrix", camViewMat * camProjMat);
		objectShader.SetUniform("cameraPosition", Camera::Active()->entity.Transform->Position());

		auto usedTextureUnits{0};

		/* Set up ambient light data */
		objectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (const auto dirLight = DataManager::DirectionalLight(); dirLight != nullptr)
		{
			objectShader.SetUniform("existsDirLight", true);
			objectShader.SetUniform("dirLight.direction", dirLight->Direction());
			objectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			objectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
			objectShader.SetUniform("dirLight.shadowMap", usedTextureUnits);
			objectShader.SetUniform("dirLightTransformMatrix", lightTransformMat.value());

			usedTextureUnits = DataManager::ShadowMaps().front().BindForReading(usedTextureUnits);
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

			objectShader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLight->entity.Transform->Position());
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

			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].position", spotLight->entity.Transform->Position());
			objectShader.SetUniform("spotLights[" + std::to_string(i) + "].direction", spotLight->entity.Transform->Forward());
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
			modelRes->DrawShaded(objectShader, matrices.first, matrices.second, usedTextureUnits);
		}
	}


	void ForwardRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat)
	{
		static auto skyboxFlagInfo{m_SkyboxShader.GetFlagInfo()};
		auto& skyboxShader{m_SkyboxShader.GetPermutation(skyboxFlagInfo)};

		if (const auto& skybox{Camera::Active()->Background().skybox}; skybox.has_value())
		{
			skyboxShader.Use();
			skyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			skyboxShader.SetUniform("projectionMatrix", camProjMat);
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(skyboxShader);
		}
	}
}
