#include "Renderer.hpp"

#include "../components/Camera.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/Light.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../instances/InstanceHolder.hpp"
#include "../math/leopphmath.h"
#include "../math/Matrix.hpp"
#include "../math/Vector.hpp"
#include "Shader.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <utility>


namespace leopph::impl
{
	Renderer::Renderer() :
		m_ObjectShader{ Shader::Type::OBJECT }, m_SkyboxShader{ Shader::Type::SKYBOX }, m_DirectionalShadowMapShader{ Shader::Type::DIRECTIONAL_SHADOW_MAP }
	{
		glEnable(GL_DEPTH_TEST);
	}


	void Renderer::Render()
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


	bool Renderer::LightLess::operator()(const Light* left, const Light* right) const
	{
		const auto& camPosition{ Camera::Active()->object.Transform().Position() };
		const auto& leftDistance{ Vector3::Distance(camPosition, left->object.Transform().Position()) };
		const auto& rightDistance{ Vector3::Distance(camPosition, right->object.Transform().Position()) };

		if (leftDistance != rightDistance)
		{
			return leftDistance < rightDistance;
		}

		return left < right;
	}


	void Renderer::CalcAndCollectModelAndNormalMatrices()
	{
		m_CurrentFrameMatrices.clear();

		for (const auto& [path, modelReference] : InstanceHolder::Models())
		{
			auto& [models, normals] = m_CurrentFrameMatrices.try_emplace(path).first->second;

			for (const auto& object : modelReference.Objects())
			{
				/* If the objet is static we query for its cached matrix */
				if (object->isStatic)
				{
					const auto& [model, normal] {InstanceHolder::ModelAndNormalMatrices(object)};
					models.emplace_back(model.Transposed());
					normals.emplace_back(normal.Transposed());
				}
				/* Otherwise we calculate it */
				else
				{
					auto modelMatrix{ Matrix4::Scale(object->Transform().Scale()) };
					modelMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
					modelMatrix *= Matrix4::Translate(object->Transform().Position());
					models.emplace_back(modelMatrix.Transposed());
					normals.emplace_back(modelMatrix.Inverse());
				}
			}
		}
	}


	void Renderer::CollectPointLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const PointLight*, LightLess> allPointsLightsOrdered;

		allPointsLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		for (const PointLight* const light : InstanceHolder::PointLights())
		{
			allPointsLightsOrdered.emplace(light);
		}

		m_CurrentFrameUsedPointLights.clear();

		/* We collect the first MAX_POINT_LIGHTS number of them */
		for (std::size_t count = 0; const auto& pointLight : allPointsLightsOrdered)
		{
			if (count == MAX_POINT_LIGHTS)
			{
				break;
			}

			m_CurrentFrameUsedPointLights.emplace_back(pointLight);
			++count;
		}
	}


	void Renderer::CollectSpotLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const SpotLight*, LightLess> allSpotLightsOrdered;

		allSpotLightsOrdered.clear();

		/* We sort the lights based on distance from camera */
		std::ranges::copy(InstanceHolder::SpotLights().begin(), InstanceHolder::SpotLights().end(), std::inserter(allSpotLightsOrdered, allSpotLightsOrdered.begin()));

		m_CurrentFrameUsedSpotLights.clear();

		/* We collect at most MAX_SPOT_LIGHTS number of them */
		const std::size_t spotLightCount{ std::min(allSpotLightsOrdered.size(), MAX_SPOT_LIGHTS) };
		std::copy_n(allSpotLightsOrdered.begin(), spotLightCount, std::back_inserter(m_CurrentFrameUsedSpotLights));
	}



	void Renderer::RenderDirectionalShadowMap()
	{
		const auto& dirLight = InstanceHolder::DirectionalLight();

		if (dirLight == nullptr)
		{
			return;
		}

		const auto& shadowMaps{ InstanceHolder::ShadowMaps() };

		/* If we don't have a shadow map, we create one */
		if (shadowMaps.empty())
		{
			InstanceHolder::CreateShadowMap(SHADOW_MAP_RESOLUTION);
		}

		const auto view{ Matrix4::LookAt(-dirLight->Direction(), Vector3{}, Vector3::Up()) };
		const auto projection{ Matrix4::Ortographic(-10, 10, 10, -10, Camera::Active()->NearClipPlane(), Camera::Active()->FarClipPlane()) };
		m_CurrentFrameDirectionalTransformMatrix = view * projection;

		m_DirectionalShadowMapShader.Use();
		shadowMaps.front().BindToBuffer();

		m_DirectionalShadowMapShader.SetUniform("lightSpaceMatrix", m_CurrentFrameDirectionalTransformMatrix);

		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			InstanceHolder::GetModelReference(modelPath).DrawDepth(matrices.first);
		}

		shadowMaps.front().UnbindFromBuffer();
	}


	void Renderer::RenderPointShadowMaps()
	{
		const auto& shadowMaps{ InstanceHolder::ShadowMaps() };

		/* If we lack the necessary number of shadow maps, we create new ones */
		while (m_CurrentFrameUsedPointLights.size() > shadowMaps.size())
		{
			InstanceHolder::CreateShadowMap(SHADOW_MAP_RESOLUTION);
		}

		/* Iterate over the lights and use a different shadow map for each */
		for (auto shadowMapIt{ InstanceHolder::DirectionalLight() == nullptr ? shadowMaps.begin() : ++shadowMaps.begin()};
			const auto & pointLight : m_CurrentFrameUsedPointLights)
		{
			for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
			{
				/*for (const auto& model{ InstanceHolder::GetModelReference(modelPath) };
					const auto & [models, normals] : matrices)
				{

				}*/
			}

			++shadowMapIt;
		}
	}


	void Renderer::RenderShadedObjects()
	{
		m_ObjectShader.Use();

		m_ObjectShader.SetUniform("viewProjectionMatrix", m_CurrentFrameViewMatrix * m_CurrentFrameProjectionMatrix);
		m_ObjectShader.SetUniform("cameraPosition", Camera::Active()->object.Transform().Position());

		std::size_t usedTextureUnits{ 0 };

		/* Set up ambient light data */
		m_ObjectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		/* Set up DirLight data */
		if (const auto dirLight = InstanceHolder::DirectionalLight(); dirLight != nullptr)
		{
			m_ObjectShader.SetUniform("existsDirLight", true);
			m_ObjectShader.SetUniform("dirLight.direction", dirLight->Direction());
			m_ObjectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_ObjectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
			m_ObjectShader.SetUniform("dirLight.shadowMap", static_cast<int>(usedTextureUnits));
			m_ObjectShader.SetUniform("dirLightTransformMatrix", m_CurrentFrameDirectionalTransformMatrix);

			InstanceHolder::ShadowMaps().front().BindToTexture(usedTextureUnits);
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
			m_ObjectShader.SetUniform("spotLights[" + std::to_string(i) + "].cutOffCosine", math::Cos(math::ToRadians(spotLight->Angle())));
		}

		/* Draw the shaded objects */
		for (const auto& [modelPath, matrices] : m_CurrentFrameMatrices)
		{
			InstanceHolder::GetModelReference(modelPath).DrawShaded(m_ObjectShader, matrices.first, matrices.second, usedTextureUnits);
		}
	}


	void Renderer::RenderSkybox() const
	{
		if (const auto& skybox{ Camera::Active()->Background().skybox }; skybox != nullptr)
		{
			m_SkyboxShader.Use();
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(m_CurrentFrameViewMatrix)));
			m_SkyboxShader.SetUniform("projectionMatrix", m_CurrentFrameProjectionMatrix);
			InstanceHolder::GetSkybox(*skybox).Draw(m_SkyboxShader);
		}
	}
}