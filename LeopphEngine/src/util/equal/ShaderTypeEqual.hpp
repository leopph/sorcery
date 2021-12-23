#pragma once

#include "../../rendering/shaders/ShaderType.hpp"


namespace leopph::internal
{
	class ShaderTypeEqual
	{
		public:
			auto operator()(ShaderType left, ShaderType right) const -> bool;
			auto operator()(const int left, const int right) const -> bool;
	};
}
