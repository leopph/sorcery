#pragma once

#include "../hierarchy/object.h"
#include <cstddef>

namespace leopph::impl
{
	class Renderer
	{
	public:
		static Renderer& Instance();
		void Render() const;

	private:
		const static std::size_t MAX_POINT_LIGHTS = 4;
		Renderer();
	};
}