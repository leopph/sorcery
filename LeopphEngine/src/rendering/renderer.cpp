#include "renderer.h"

#include "../components/camera.h"
#include "../components/lighting/dirlight.h"
#include "../instances/instanceholder.h"
#include "../components/lighting/light.h"
#include "../math/matrix.h"
#include "../components/lighting/pointlight.h"
#include "shader.h"
#include "../math/vector.h"

#include <glad/glad.h>

#include <string>

namespace leopph::impl
{
	Renderer::Renderer() :
		m_Shader{ Shader::Type::GENERAL }, m_SkyboxShader{ Shader::Type::SKYBOX }
	{
		glEnable(GL_DEPTH_TEST);
	}


	void Renderer::Render() const
	{
		if (Camera::Active() == nullptr)
			return;

		auto viewMatrix{ Camera::Active()->ViewMatrix() };
		auto projectionMatrix{ Camera::Active()->ProjectionMatrix() };

		PointLight* pointLights[MAX_POINT_LIGHTS]{};

		for (Light* light : InstanceHolder::PointLights())
		{
			if (pointLights[0] == nullptr)
			{
				pointLights[0] = reinterpret_cast<PointLight*>(light);
			}
			else
			{
				light = reinterpret_cast<PointLight*>(light);

				Vector3 lightPos = light->Object().Transform().Position();
				Vector3 camPos = Camera::Active()->Object().Transform().Position();

				if (Vector3::Distance(lightPos, camPos) < Vector3::Distance(camPos, pointLights[0]->Object().Transform().Position()))
				{
					for (size_t i = MAX_POINT_LIGHTS - 1; i > 0; i--)
						pointLights[i] = pointLights[i - 1];

					pointLights[0] = reinterpret_cast<PointLight*>(light);
				}
			}
		}

		m_Shader.Use();

		size_t lightNumber = 0;

		for (auto& pointLight : pointLights)
		{
			if (pointLight != nullptr)
			{
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].position", static_cast<Vector3>(static_cast<Vector4>(pointLight->Object().Transform().Position()) * viewMatrix));
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].diffuseColor", pointLight->Diffuse());
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].specularColor", pointLight->Specular());
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].constant", pointLight->Constant());
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].linear", pointLight->Linear());
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].quadratic", pointLight->Quadratic());

				lightNumber++;
			}
		}
		m_Shader.SetUniform("lightNumber", static_cast<int>(lightNumber));

		if (const auto dirLight = InstanceHolder::DirectionalLight(); dirLight != nullptr)
		{
			m_Shader.SetUniform("existsDirLight", true);
			m_Shader.SetUniform("dirLight.direction", dirLight->Direction() * static_cast<Matrix3>(viewMatrix));
			m_Shader.SetUniform("dirLight.diffuseColor", dirLight->Diffuse());
			m_Shader.SetUniform("dirLight.specularColor", dirLight->Specular());
		}
		else
		{
			m_Shader.SetUniform("existsDirLight", false);
		}

		m_Shader.SetUniform("ambientLight", AmbientLight::Instance().Intensity());

		m_Shader.SetUniform("projectionMatrix", projectionMatrix);

		for (const auto& pair : InstanceHolder::Models())
		{
			std::vector<Matrix4> modelViewMatrices;
			std::vector<Matrix4> normalMatrices;
			const auto& modelReference = pair.second;

			for (const auto& object : modelReference.Objects())
			{
				Matrix4 modelViewMatrix{ 1.0f };
				modelViewMatrix *= Matrix4::Scale(object->Transform().Scale());
				modelViewMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
				modelViewMatrix *= Matrix4::Translate(object->Transform().Position());
				modelViewMatrix *= viewMatrix;

				modelViewMatrices.push_back(modelViewMatrix.Transposed());
				normalMatrices.push_back(modelViewMatrix.Inverse());
			}

			modelReference.ReferenceModel().Draw(m_Shader, modelViewMatrices, normalMatrices);
		}

		m_SkyboxShader.Use();

		if (const auto& skybox{ Camera::Active()->Background().skybox }; skybox != nullptr)
		{
			m_SkyboxShader.SetUniform("viewMatrix", static_cast<Matrix4>(static_cast<Matrix3>(viewMatrix)));
			m_SkyboxShader.SetUniform("projectionMatrix", projectionMatrix);
			InstanceHolder::GetSkybox(*skybox).Draw(m_SkyboxShader);
		}
	}
}