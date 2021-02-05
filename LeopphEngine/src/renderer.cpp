#include "renderer.h"
#include "shader.h"
#include "object.h"
#include "light.h"
#include "pointlight.h"

namespace leopph::implementation
{
	Renderer& Renderer::Instance()
	{
		static Renderer instance;
		return instance;
	}


	void Renderer::Render()
	{
		static Shader shader{ "shaders/vertex.vert", "shaders/fragment.frag" };

		Light* lights[MAX_POINT_LIGHTS];

		for (Light* light : Light::PointLights())
		{
			light = reinterpret_cast<PointLight*>(light);
		}

		shader.Use();
	}
}