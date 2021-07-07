#pragma once

#include "../hierarchy/object.h"
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

		const static std::size_t MAX_POINT_LIGHTS = 64;
	};
}