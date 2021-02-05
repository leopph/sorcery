#include "renderer.h"
#include "shader.h"
#include "object.h"
#include "light.h"
#include "pointlight.h"
#include "leopphmath.h"
#include "camera.h"
#include "matrix.h"
#include <string>

using std::size_t;

namespace leopph::implementation
{
	Renderer& Renderer::Instance()
	{
		static Renderer instance;
		return instance;
	}


	void Renderer::Render()
	{
		static Shader shader{ "./shaders/vertex.vert", "./shaders/fragment.frag" };

		PointLight* pointLights[MAX_POINT_LIGHTS]{};

		for (Light* light : Light::PointLights())
		{
			if (pointLights[0] == nullptr)
				pointLights[0] = reinterpret_cast<PointLight*>(light);

			else
			{

				light = reinterpret_cast<PointLight*>(light);

				Vector3 lightPos = light->OwningObject().Position();
				Vector3 camPos = Camera::Instance().Position();

				if (Vector3::Distance(lightPos, camPos) < Vector3::Distance(camPos, pointLights[0]->OwningObject().Position()))
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
				lightNumber++;

				shader.SetUniform("pointLights[" + std::to_string(i) + "].position", pointLights[i]->OwningObject().Position());
				shader.SetUniform("pointLights[" + std::to_string(i) + "].ambient", pointLights[i]->Ambient());
				shader.SetUniform("pointLights[" + std::to_string(i) + "].diffuse", pointLights[i]->Diffuse());
				shader.SetUniform("pointLights[" + std::to_string(i) + "].specular", pointLights[i]->Specular());
				shader.SetUniform("pointLights[" + std::to_string(i) + "].constant", pointLights[i]->Constant());
				shader.SetUniform("pointLights[" + std::to_string(i) + "].linear", pointLights[i]->Linear());
				shader.SetUniform("pointLights[" + std::to_string(i) + "].quadratic", pointLights[i]->Quadratic());
			}

		shader.SetUniform("lightNumber", static_cast<int>(lightNumber));
		shader.SetUniform("view", Camera::Instance().ViewMatrix());
		shader.SetUniform("proj", Camera::Instance().ProjMatrix());

		for (const auto& object : Object::Instances())
			for (const auto& model : object->Models())
			{
				Matrix4 modelMatrix = Matrix4::Identity();

				// TODO
				// translate by object pos
				// rotate by object rotation
				// scale by object scale

				shader.SetUniform("model", modelMatrix);
				shader.SetUniform("normalMatrix", modelMatrix); // TODO IMPLEMENT INVERSE AND INVERT MODEL

				model.Draw(shader);
			}
	}
}