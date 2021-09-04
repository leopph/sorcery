#pragma once

#include "../../entity/Entity.hpp"

#include <cstddef>
#include <functional>
#include <string>


namespace leopph::impl
{
	class EntityHash
	{
	public:
		using is_transparent = void;

		EntityHash();

		std::size_t operator()(const Entity* entity) const;
		std::size_t operator()(const std::string& str) const;


	private:
		std::hash<std::string> m_Hash;
	};
}