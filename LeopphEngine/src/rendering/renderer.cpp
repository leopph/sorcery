#include "renderer.h"
#include "shader.h"
#include "../components/lighting/light.h"
#include "../components/lighting/pointlight.h"
#include "../components/lighting/dirlight.h"
#include "../components/camera.h"
#include "../math/matrix.h"
#include "../instances/instanceholder.h"
#include <string>
#include <glad/glad.h>

using std::size_t;

namespace leopph::impl
{
	Renderer::Renderer()
	{
		glEnable(GL_DEPTH_TEST);
	}


	Renderer& Renderer::Instance()
	{
		static Renderer instance;
		return instance;
	}


	void Renderer::Render() const
	{
		static Shader shader{ "./shaders/vertex.vert", "./shaders/fragment.frag" };

		if (Camera::Active() == nullptr)
			return;


		PointLight* pointLights[MAX_POINT_LIGHTS]{};

		for (Light* light : InstanceHolder::PointLights())
		{
			if (pointLights[0] == nullptr)
				pointLights[0] = reinterpret_cast<PointLight*>(light);

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



		shader.Use();



		size_t lightNumber = 0;

		for (size_t i = 0; i < MAX_POINT_LIGHTS; i++)
			if (pointLights[i] != nullptr)
			{
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].position", pointLights[i]->Object().Transform().Position());
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].ambient", pointLights[i]->Ambient());
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].diffuse", pointLights[i]->Diffuse());
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].specular", pointLights[i]->Specular());
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].constant", pointLights[i]->Constant());
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].linear", pointLights[i]->Linear());
				shader.SetUniform("pointLights[" + std::to_string(lightNumber) + "].quadratic", pointLights[i]->Quadratic());

				lightNumber++;
			}
		shader.SetUniform("lightNumber", static_cast<int>(lightNumber));




		if (const auto dirLight = InstanceHolder::DirectionalLight(); dirLight != nullptr)
		{
			shader.SetUniform("existsDirLight", true);
			shader.SetUniform("dirLight.direction", dirLight->Direction());
			shader.SetUniform("dirLight.ambient", dirLight->Ambient());
			shader.SetUniform("dirLight.diffuse", dirLight->Diffuse());
			shader.SetUniform("dirLight.specular", dirLight->Specular());
		}
		else
			shader.SetUniform("existsDirLight", false);




		shader.SetUniform("viewPosition", Camera::Active()->Object().Transform().Position());
		shader.SetUniform("view", Camera::Active()->ViewMatrix());
		shader.SetUniform("proj", Camera::Active()->ProjectionMatrix());



		for (const auto& object : InstanceHolder::Objects())
			for (const auto& model : object->Models())
			{
				Matrix4 modelMatrix{ 1.0f };
				modelMatrix *= Matrix4::Scale(object->Transform().Scale());
				modelMatrix *= static_cast<Matrix4>(object->Transform().Rotation());
				modelMatrix *= Matrix4::Translate(object->Transform().Position());

				shader.SetUniform("model", modelMatrix);
				shader.SetUniform("normalMatrix", modelMatrix.Inverse().Transposed());

				model.Draw(shader);
			}
	}
}