#pragma once

#include "leopphapi.h"
#include "object.h"
#include <cstddef>

namespace leopph::implementation
{
	class LEOPPHAPI Renderer
	{
	private:
		Renderer();

		const static std::size_t MAX_POINT_LIGHTS = 4;

	public:
		static Renderer& Instance();

		void Render();
	};
}