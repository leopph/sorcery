#include "EntityEqual.hpp"


namespace leopph::internal
{
	bool EntityEqual::operator()(const Entity* left, const Entity* right) const
	{
		return left->Name() == right->Name();
	}


	bool EntityEqual::operator()(const std::string& left, const Entity* right) const
	{
		return left == right->Name();
	}


	bool EntityEqual::operator()(const Entity* left, const std::string& right) const
	{
		return left->Name() == right;
	}
}