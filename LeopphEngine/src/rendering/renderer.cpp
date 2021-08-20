#include "Renderer.hpp"

#include "../components/Camera.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/Light.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../instances/InstanceHolder.hpp"
#include "../math/matrix.h"
#include "../math/vector.h"
#include "Shader.hpp"

#include <glad/glad.h>

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
		CollectModelMatrices();

		/* We collect the nearest pointlights */
		CollectPointLights();

		/* We store the main camera's view and projection matrices for the frame */
		m_CurrentFrameViewMatrix = Camera::Active()->ViewMatrix();
		m_CurrentFrameProjectionMatrix = Camera::Active()->ProjectionMatrix();

		//RenderDirectionalShadowMap();
		//RenderPointShadowMaps();
		RenderShadedObjects();
		RenderSkybox();

		
	}


	bool Renderer::PointLightLess::operator()(const PointLight* left, const PointLight* right) const
	{
		const auto camPosition{ Camera::Active()->object.Transform().Position() };
		const auto leftDistance{ Vector3::Distance(camPosition, left->object.Transform().Position()) };
		const auto rightDistance{ Vector3::Distance(camPosition, right->object.Transform().Position()) };

		if (leftDistance != rightDistance)
		{
			return leftDistance < rightDistance;
		}

		return left < right;
	}


	void Renderer::CollectModelMatrices()
	{
		m_CurrentFrameModelMatrices.clear();

		for (const auto& [path, modelReference] : InstanceHolder::Models())
		{
			auto& matrices = m_CurrentFrameModelMatrices.try_emplace(path).first->second;

			for (const auto& object : modelReference.Objects())
			{
				/* If the objet is static we query for its cached matrix */
				if (object->isStatic)
				{
					matrices.emplace_back(InstanceHolder::ModelMatrix(object));
				}
				/* Otherwise we calculate it */
				else
				{
					auto modelMatrix{ Matrix4::Scale(object->Transform().Scale()) };
					modelMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
					modelMatrix *= Matrix4::Translate(object->Transform().Position());
					matrices.emplace_back(modelMatrix);
				}
			}
		}
	}


	void Renderer::CollectPointLights()
	{
		/* This set stores lights in an ascending order based on distance from camera */
		static std::set<const PointLight*, PointLightLess> allPointsLightsOrdered;

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


	void Renderer::RenderDirectionalShadowMap() const
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

		const auto projection{ Matrix4::LookAt(-dirLight->Direction(), Vector3{}, Vector3::Up()) };
		const auto view{ Matrix4::Ortographic(-10, 10, -10, 10, Camera::Active()->NearClipPlane(), Camera::Active()->FarClipPlane()) };

		m_DirectionalShadowMapShader.Use();
		shadowMaps.front().Bind();

		m_DirectionalShadowMapShader.SetUniform("lightSpaceMatrix", view * projection);

		for (const auto& [modelPath, matrices] : m_CurrentFrameModelMatrices)
		{
			InstanceHolder::GetModelReference(modelPath).DrawDepth(m_DirectionalShadowMapShader, matrices);
		}

		shadowMaps.front().Unbind();
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
			for (const auto& [modelPath, matrices] : m_CurrentFrameModelMatrices)
			{
				for (const auto& model{ InstanceHolder::GetModelReference(modelPath) };
					const auto & matrix : matrices)
				{

				}
			}

			++shadowMapIt;
		}
	}


	void Renderer::RenderShadedObjects()
	{
		m_ObjectShader.Use();

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

		m_ObjectShader.SetUniform("lightNumber", static_cast<int>(m_CurrentFrameUsedPointLights.size()));

		if (const auto dirLight = InstanceHolder::DirectionalLight(); dirLight != nullptr)
		{
			m_ObjectShader.SetUniform("existsDirLight", true);
			m_ObjectShader.SetUniform("dirLight.direction", dirLight->Direction());
			m_ObjectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_ObjectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
		}
		else
		{
			m_ObjectShader.SetUniform("existsDirLight", false);
		}

		m_ObjectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		m_ObjectShader.SetUniform("viewMatrix", m_CurrentFrameViewMatrix);
		m_ObjectShader.SetUniform("projectionMatrix", m_CurrentFrameProjectionMatrix);

		m_ObjectShader.SetUniform("cameraPosition", Camera::Active()->object.Transform().Position());

		std::vector<Matrix4> normalMatrices;
		for (const auto& [modelPath, matrices] : m_CurrentFrameModelMatrices)
		{
			normalMatrices.clear();

			for (const auto& matrix : matrices)
			{
				normalMatrices.push_back(matrix.Inverse());
			}

			InstanceHolder::GetModelReference(modelPath).DrawShaded(m_ObjectShader, matrices, normalMatrices);
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