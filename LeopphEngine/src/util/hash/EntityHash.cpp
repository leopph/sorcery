#include "EntityHash.hpp"

#include <functional>


namespace leopph::internal
{
	auto EntityHash::operator()(const Entity* entity) const -> std::size_t
	{
		return m_Hash(entity->Name());
	}

	auto EntityHash::operator()(const std::string& str) const -> std::size_t
	{
		return m_Hash(str);
	}
}
