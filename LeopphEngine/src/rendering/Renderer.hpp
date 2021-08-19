#pragma once

#include "shader.h"

#include "../components/lighting/PointLight.hpp"

#include <cstddef>
#include <functional>
#include <set>


namespace leopph::impl
{
	class Renderer
	{
	public:
		Renderer();
		void Render();

	private:
		Shader m_ObjectShader;
		Shader m_SkyboxShader;

		constexpr static std::size_t MAX_POINT_LIGHTS = 64;
		std::set<const PointLight*, std::function<bool(const PointLight*, const PointLight*)>> m_PointsLights;
	};
}