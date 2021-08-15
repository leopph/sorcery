#pragma once

#include "shader.h"

#include <cstddef>

namespace leopph::impl
{
	class Renderer
	{
	public:
		Renderer();
		void Render() const;

	private:
		Shader m_Shader;
		Shader m_SkyboxShader;

		constexpr static std::size_t MAX_POINT_LIGHTS = 64;
	};
}