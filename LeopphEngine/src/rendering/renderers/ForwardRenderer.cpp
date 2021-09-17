#include "ForwardRenderer.hpp"

#include "../SkyboxResource.hpp"
#include "../../components/Camera.hpp"
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

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <set>
#include <string>
#include <utility>



namespace leopph::impl
{
	ForwardRenderer::ForwardRenderer()
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


	std::optional<Matrix4> ForwardRenderer::RenderDirectionalShadowMap(const std::unordered_map<const ModelResource*, std::pair<std::vector<Matrix4>, std::vector<Matrix4>>>& modelsAndMats) const
	{
		const auto& dirLight = DataManager::DirectionalLight();

		if (dirLight == nullptr)
		{
			return {};
		}

		const auto& shadowMaps{DataManager::ShadowMaps()};

		/* If we don't have a shadow map, we create one */
		if (shadowMaps.empty())
		{
			DataManager::CreateShadowMap(Settings::DirectionalShadowMapResolutions()[0]); // TODO cascades
		}

		const auto lightViewMat{Matrix4::LookAt(-dirLight->Direction(), Vector3{}, Vector3::Up())};
		const auto lightProjMat{Matrix4::Ortographic(-10, 10, 10, -10, Camera::Active()->NearClipPlane(), Camera::Active()->FarClipPlane())};
		const auto lightTransformMat{lightViewMat * lightProjMat};

		m_ShadowShader.Use();
		shadowMaps.front().BindForWriting();

		m_ShadowShader.SetLightWorldToClipMatrix(lightTransformMat);

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
											  const std::vector<const SpotLight*>& spotLights) const
	{
		m_ObjectShader.Use();

		m_ObjectShader.SetUniform("viewProjectionMatrix", camViewMat * camProjMat);
		m_ObjectShader.SetUniform("cameraPosition", Camera::Active()->entity.Transform->Position());

		auto usedTextureUnits{0};

		/* Set up ambient light data */
		m_ObjectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (const auto dirLight = DataManager::DirectionalLight(); dirLight != nullptr)
		{
			m_ObjectShader.SetUniform("existsDirLight", true);
			m_ObjectShader.SetUniform("dirLight.direction", dirLight->Direction());
			m_ObjectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_ObjectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
			m_ObjectShader.SetUniform("dirLight.shadowMap", usedTextureUnits);
			m_ObjectShader.SetUniform("dirLightTransformMatrix", lightTransformMat.value());

			usedTextureUnits = DataManager::ShadowMaps().front().BindForReading(usedTextureUnits);
		}
		else
		{
			m_ObjectShader.SetUniform("existsDirLight", false);
		}

		/* Set up PointLight data */
		m_ObjectShader.SetUniform("pointLightCount", static_cast<int>(pointLights.size()));
		for (std::size_t i = 0; i < pointLights.size(); i++)
		{
			const auto& pointLight = pointLights[i];

			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLight->entity.Transform->Position());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
		}

		/* Set up SpotLight data */
		m_ObjectShader.SetUniform("spotLightCount", static_cast<int>(spotLights.size()));
		for (std::size_t i = 0; i < spotLights.size(); i++)
		{
			const auto& spotLight{spotLights[i]};

			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].position", spotLight->entity.Transform->Position());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].direction", spotLight->entity.Transform->Forward());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		/* Draw the shaded objects */
		for (const auto& [modelRes, matrices] : modelsAndMats)
		{
			modelRes->DrawShaded(m_ObjectShader, matrices.first, matrices.second, usedTextureUnits);
		}
	}


	void ForwardRenderer::RenderSkybox(const Matrix4& camViewMat, const Matrix4& camProjMat) const
	{
		if (const auto& skybox{Camera::Active()->Background().skybox}; skybox.has_value())
		{
			m_SkyboxShader.Use();
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(camViewMat)));
			m_SkyboxShader.SetUniform("projectionMatrix", camProjMat);
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(m_SkyboxShader);
		}
	}
}
