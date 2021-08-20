#include "Renderer.hpp"

#include "../components/Camera.hpp"
#include "../components/lighting/DirLight.hpp"
#include "../instances/InstanceHolder.hpp"
#include "../components/lighting/Light.hpp"
#include "../math/matrix.h"
#include "../components/lighting/PointLight.hpp"
#include "shader.h"
#include "../math/vector.h"

#include <glad/glad.h>

#include <string>
#include <utility>

namespace leopph::impl
{
	Renderer::Renderer() :
		m_ObjectShader{ Shader::Type::GENERAL }, m_SkyboxShader{ Shader::Type::SKYBOX }
	{
		glEnable(GL_DEPTH_TEST);
	}


	void Renderer::Render()
	{
		if (Camera::Active() == nullptr)
			return;

		const auto viewMatrix{ Camera::Active()->ViewMatrix() };
		const auto projectionMatrix{ Camera::Active()->ProjectionMatrix() };

		m_PointsLights.clear();

		for (const PointLight* const light : InstanceHolder::PointLights())
		{
			m_PointsLights.emplace(light);
		}

		m_ObjectShader.Use();

		size_t lightNumber = 0;

		for (const auto& pointLight : m_PointsLights)
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
		}

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

}