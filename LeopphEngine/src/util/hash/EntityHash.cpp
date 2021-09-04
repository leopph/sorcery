#include "EntityHash.hpp"

#include <functional>


namespace leopph::impl
{
	EntityHash::EntityHash() :
		m_Hash{}
	{
	}


	std::size_t EntityHash::operator()(const Entity* entity) const
	{
		return m_Hash(entity->name);
	}


	std::size_t EntityHash::operator()(const std::string& str) const
	{
		return m_Hash(str);
	}
}