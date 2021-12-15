#include "EntityHash.hpp"

#include <functional>


namespace leopph::impl
{
	std::size_t EntityHash::operator()(const Entity* entity) const
	{
		return m_Hash(entity->Name);
	}


	std::size_t EntityHash::operator()(const std::string& str) const
	{
		return m_Hash(str);
	}
}