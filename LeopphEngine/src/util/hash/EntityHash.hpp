#pragma once

#include "../../entity/Entity.hpp"

#include <cstddef>
#include <functional>
#include <string>


namespace leopph::internal
{
	class EntityHash
	{
		public:
			using is_transparent = void;

			auto operator()(const Entity* entity) const -> std::size_t;
			auto operator()(const std::string& str) const -> std::size_t;

		private:
			std::hash<std::string> m_Hash;
	};
}
