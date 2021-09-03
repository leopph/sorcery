#include "ForwardRenderer.hpp"

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
#include "../Shader.hpp"
#include "../SkyboxResource.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <set>
#include <string>
#include <utility>


namespace leopph::impl
{
	ForwardRenderer::ForwardRenderer() :
		m_ObjectShader{ Shader::Type::OBJECT }, m_SkyboxShader{ Shader::Type::SKYBOX }, m_DirectionalShadowMapShader{ Shader::Type::DIRECTIONAL_SHADOW_MAP }
	{
		glEnable(GL_DEPTH_TEST);
	}


	void ForwardRenderer::Render()
	{
		/* We don't render if there is no camera to use */
		if (Camera::Active() == nullptr)
			return;

		/* We collect the model matrices from the existing models' objects */
		CalcAndCollectModelAndNormalMatrices();

		/* We collect the nearest pointlights */
		CollectPointLights();

		/* We collect the nearest spotlights */
		CollectSpotLights();

		/* We store the main camera's view and projection matrices for the frame */
		m_CurrentFrameViewMatrix = Camera::Active()->ViewMatrix();
		m_CurrentFrameProjectionMatrix = Camera::Active()->ProjectionMatrix();

		RenderDirectionalShadowMap();
		//RenderPointShadowMaps();
		RenderShadedObjects();
		RenderSkybox();
	}


	void ForwardRenderer::RenderDirectionalShadowMap()
	{
		const auto& dirLight = DataManager::DirectionalLight();

		if (dirLight == nullptr)
		{
			return;
		}

		const auto& shadowMaps{ DataManager::ShadowMaps() };

		/* If we don't have a shadow map, we create one */
		if (shadowMaps.empty())
		{
			DataManager::CreateShadowMap(Settings::DirectionalLightShadowMapResolution());
		}

		const auto view{ Matrix4::LookAt(-dirLight->Direction(), Vector3{}, Vector3::Up()) };
		const auto projection{ Matrix4::Ortographic(-10, 10, 10, -10, Camera::Active()->NearClipPlane(), Camera::Active()->FarClipPlane()) };
		m_CurrentFrameDirectionalTransformMatrix = view * projection;

		m_DirectionalShadowMapShader.Use();
		shadowMaps.front().BindToBuffer();

		m_DirectionalShadowMapShader.SetUniform("lightSpaceMatrix", m_CurrentFrameDirectionalTransformMatrix);

		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			static_cast<ModelResource*>(DataManager::Find(modelPath))->DrawDepth(matrices.first);
		}

		shadowMaps.front().UnbindFromBuffer();
	}


	void ForwardRenderer::RenderPointShadowMaps()
	{
		const auto& shadowMaps{ DataManager::ShadowMaps() };

		/* If we lack the necessary number of shadow maps, we create new ones */
		while (m_CurrentFrameUsedPointLights.size() > shadowMaps.size())
		{
			DataManager::CreateShadowMap(Settings::PointLightShadowMapResolution());
		}

		/* Iterate over the lights and use a different shadow map for each */
		for (auto shadowMapIt{ DataManager::DirectionalLight() == nullptr ? shadowMaps.begin() : ++shadowMaps.begin()};
			const auto & pointLight : m_CurrentFrameUsedPointLights)
		{
			for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
			{
				static_cast<ModelResource*>(DataManager::Find(modelPath))->DrawDepth(matrices.first);
			}

			++shadowMapIt;
		}
	}


	void ForwardRenderer::RenderShadedObjects()
	{
		m_ObjectShader.Use();

		m_ObjectShader.SetUniform("viewProjectionMatrix", m_CurrentFrameViewMatrix * m_CurrentFrameProjectionMatrix);
		m_ObjectShader.SetUniform("cameraPosition", Camera::Active()->object.Transform().Position());

		std::size_t usedTextureUnits{ 0 };

		/* Set up ambient light data */
		m_ObjectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (const auto dirLight = DataManager::DirectionalLight(); dirLight != nullptr)
		{
			m_ObjectShader.SetUniform("existsDirLight", true);
			m_ObjectShader.SetUniform("dirLight.direction", dirLight->Direction());
			m_ObjectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_ObjectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
			m_ObjectShader.SetUniform("dirLight.shadowMap", static_cast<int>(usedTextureUnits));
			m_ObjectShader.SetUniform("dirLightTransformMatrix", m_CurrentFrameDirectionalTransformMatrix);

			DataManager::ShadowMaps().front().BindToTexture(usedTextureUnits);
			++usedTextureUnits;
		}
		else
		{
			m_ObjectShader.SetUniform("existsDirLight", false);
		}

		/* Set up PointLight data */
		m_ObjectShader.SetUniform("pointLightCount", static_cast<int>(m_CurrentFrameUsedPointLights.size()));
		for (std::size_t i = 0; i < m_CurrentFrameUsedPointLights.size(); i++)
		{
			const auto& pointLight = m_CurrentFrameUsedPointLights[i];

			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLight->object.Transform().Position());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].diffuseColor", pointLight->Diffuse());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].specularColor", pointLight->Specular());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].constant", pointLight->Constant());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].linear", pointLight->Linear());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(i) + "].quadratic", pointLight->Quadratic());
		}

		/* Set up SpotLight data */
		m_ObjectShader.SetUniform("spotLightCount", static_cast<int>(m_CurrentFrameUsedSpotLights.size()));
		for (std::size_t i = 0; i < m_CurrentFrameUsedSpotLights.size(); i++)
		{
			const auto& spotLight{ m_CurrentFrameUsedSpotLights[i] };

			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].position", spotLight->object.Transform().Position());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].direction", spotLight->object.Transform().Forward());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].diffuseColor", spotLight->Diffuse());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].specularColor", spotLight->Specular());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].constant", spotLight->Constant());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].linear", spotLight->Linear());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].quadratic", spotLight->Quadratic());
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].innerAngleCosine", math::Cos(math::ToRadians(spotLight->InnerAngle())));
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].outerAngleCosine", math::Cos(math::ToRadians(spotLight->OuterAngle())));
		}

		/* Draw the shaded objects */
		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			static_cast<ModelResource*>(DataManager::Find(modelPath))->DrawShaded(m_ObjectShader, matrices.first, matrices.second, usedTextureUnits);
		}
	}


	void ForwardRenderer::RenderSkybox() const
	{
		if (const auto& skybox{ Camera::Active()->Background().skybox }; skybox != nullptr)
		{
			m_SkyboxShader.Use();
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(m_CurrentFrameViewMatrix)));
			m_SkyboxShader.SetUniform("projectionMatrix", m_CurrentFrameProjectionMatrix);
			static_cast<SkyboxResource*>(DataManager::Find(skybox->AllFilePaths()))->Draw(m_SkyboxShader);
		}
	}
}