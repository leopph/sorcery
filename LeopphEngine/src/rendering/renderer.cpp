#include "Renderer.hpp"

#include "../components/Camera.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../components/lighting/Light.hpp"
#include "../components/lighting/PointLight.hpp"
#include "../instances/InstanceHolder.hpp"
#include "../math/matrix.h"
#include "../math/vector.h"
#include "shader.h"

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

		const auto viewMatrix{ Camera::Active()->ViewMatrix() };
		const auto projectionMatrix{ Camera::Active()->ProjectionMatrix() };



		m_ObjectShader.Use();

		size_t lightNumber = 0;

		/*for (const auto& pointLight : m_AllPointsLightsOrdered)
		{
			if (lightNumber == MAX_POINT_LIGHTS)
			{
				break;
			}

			m_ObjectShader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].position", static_cast<Vector3>(static_cast<Vector4>(pointLight->object.Transform().Position()) * viewMatrix));
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].diffuseColor", pointLight->Diffuse());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].specularColor", pointLight->Specular());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].constant", pointLight->Constant());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].linear", pointLight->Linear());
			m_ObjectShader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].quadratic", pointLight->Quadratic());

			lightNumber++;
		}*/

		m_ObjectShader.SetUniform("lightNumber", static_cast<int>(lightNumber));

		if (const auto dirLight = InstanceHolder::DirectionalLight(); dirLight != nullptr)
		{
			m_ObjectShader.SetUniform("existsDirLight", true);
			m_ObjectShader.SetUniform("dirLight.direction", dirLight->Direction() * static_cast<Matrix3>(viewMatrix));
			m_ObjectShader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_ObjectShader.SetUniform("dirLight.specularColor", dirLight->Specular());
		}
		else
		{
			m_ObjectShader.SetUniform("existsDirLight", false);
		}

		m_ObjectShader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		m_ObjectShader.SetUniform("projectionMatrix", projectionMatrix);

		for (const auto& [path, modelReference] : InstanceHolder::Models())
		{
			std::vector<Matrix4> modelViewMatrices;
			std::vector<Matrix4> normalMatrices;

			for (const auto& object : modelReference.Objects())
			{
				if (object->isStatic)
				{
					const auto& modelViewMatrix = InstanceHolder::ModelMatrix(object) * viewMatrix;
					modelViewMatrices.push_back(modelViewMatrix.Transposed());
					normalMatrices.push_back(modelViewMatrix.Inverse());
				}
				else
				{
					Matrix4 modelViewMatrix{ 1.0f };
					modelViewMatrix *= Matrix4::Scale(object->Transform().Scale());
					modelViewMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
					modelViewMatrix *= Matrix4::Translate(object->Transform().Position());
					modelViewMatrix *= viewMatrix;

					modelViewMatrices.push_back(modelViewMatrix.Transposed());
					normalMatrices.push_back(modelViewMatrix.Inverse());
				}
			}

			modelReference.ReferenceModel().Draw(m_ObjectShader, modelViewMatrices, normalMatrices);
		}

		m_SkyboxShader.Use();

		if (const auto& skybox{ Camera::Active()->Background().skybox }; skybox != nullptr)
		{
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(viewMatrix)));
			m_SkyboxShader.SetUniform("projectionMatrix", projectionMatrix);
			InstanceHolder::GetSkybox(*skybox).Draw(m_SkyboxShader);
		}
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

	void Renderer::RenderShadowMaps()
	{
		for (const auto& pointLight : m_CurrentFrameUsedPointLights)
		{
			for (const auto& [modelPath, matrices] : m_CurrentFrameModelMatrices)
			{
				
			}
		}
	}

}