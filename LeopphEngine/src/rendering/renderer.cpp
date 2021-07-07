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
	Renderer::Renderer()
	{
		glEnable(GL_DEPTH_TEST);
	}


	void Renderer::Render() const
	{
		if (Camera::Active() == nullptr)
			return;

		const auto viewMatrix{ Camera::Active()->ViewMatrix() };

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
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].position", static_cast<Vector4>(pointLight->Object().Transform().Position()) * viewMatrix);
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].ambient", pointLight->Ambient());
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].diffuse", pointLight->Diffuse());
				m_Shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].specular", pointLight->Specular());
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
			m_Shader.SetUniform("dirLight.direction", (static_cast<Vector4>(dirLight->Direction()) * viewMatrix).Normalize());
			m_Shader.SetUniform("dirLight.ambient", dirLight->Ambient());
			m_Shader.SetUniform("dirLight.diffuse", dirLight->Diffuse());
			m_Shader.SetUniform("dirLight.specular", dirLight->Specular());
		}
		else
		{
			m_Shader.SetUniform("existsDirLight", false);
		}

		m_Shader.SetUniform("projectionMatrix", Camera::Active()->ProjectionMatrix());

		for (const auto& pair : InstanceHolder::Models())
		{
			for (const auto& modelReference = pair.second; const auto& object : modelReference.Objects())
			{
				Matrix4 modelViewMatrix{ 1.0f };
				modelViewMatrix *= Matrix4::Scale(object->Transform().Scale());
				modelViewMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
				modelViewMatrix *= Matrix4::Translate(object->Transform().Position());
				modelViewMatrix *= viewMatrix;

				m_Shader.SetUniform("modelViewMatrix", modelViewMatrix);
				m_Shader.SetUniform("normalMatrix", modelViewMatrix.Inverse().Transposed());

				modelReference.ReferenceModel().Draw(m_Shader);
			}
		}
	}
}