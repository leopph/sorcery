#pragma once

#include "../../rendering/shaders/ShaderType.hpp"

#include <cstddef>
#include <functional>



namespace leopph::internal
{
	class ShaderTypeHash
	{
		public:
			std::size_t operator()(ShaderType type) const;
			std::size_t operator()(const int i) const;


		private:
			std::hash<int> m_Hash;
	};
}
