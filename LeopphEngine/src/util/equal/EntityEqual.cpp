#include "EntityEqual.hpp"


namespace leopph::impl
{
	bool EntityEqual::operator()(const Entity* left, const Entity* right) const
	{
		return left->name == right->name;
	}


	bool EntityEqual::operator()(const std::string& left, const Entity* right) const
	{
		return left == right->name;
	}


	bool EntityEqual::operator()(const Entity* left, const std::string& right) const
	{
		return left->name == right;
	}
}