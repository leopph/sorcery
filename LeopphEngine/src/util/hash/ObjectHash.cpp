#include "ObjectHash.hpp"

#include <functional>


namespace leopph::impl
{
	ObjectHash::ObjectHash() :
		m_Hash{}
	{
	}


	std::size_t ObjectHash::operator()(const Object* object) const
	{
		return m_Hash(object->name);
	}


	std::size_t ObjectHash::operator()(const std::string& str) const
	{
		return m_Hash(str);
	}
}