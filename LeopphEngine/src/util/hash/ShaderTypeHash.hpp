#pragma once

#include "../../rendering/shaders/ShaderType.hpp"

#include <cstddef>
#include <functional>


namespace leopph::internal
{
	class ShaderTypeHash
	{
		public:
			auto operator()(ShaderType type) const -> std::size_t;
			auto operator()(const int i) const -> std::size_t;

		private:
			std::hash<int> m_Hash;
	};
}
