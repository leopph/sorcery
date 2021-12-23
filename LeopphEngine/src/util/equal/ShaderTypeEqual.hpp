#pragma once

#include "../../rendering/shaders/ShaderType.hpp"



namespace leopph::internal
{
	class ShaderTypeEqual
	{
		public:
			bool operator()(ShaderType left, ShaderType right) const;
			bool operator()(const int left, const int right) const;
	};
}
