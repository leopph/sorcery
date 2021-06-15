#pragma once

#include "../hierarchy/object.h"
#include <cstddef>

namespace leopph::impl
{
	class Renderer
	{
	private:
		Renderer();

		const static std::size_t MAX_POINT_LIGHTS = 4;

	public:
		static Renderer& Instance();

		void Render();
	};
}