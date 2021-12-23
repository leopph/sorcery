#include "EntityEqual.hpp"


namespace leopph::internal
{
	auto EntityEqual::operator()(const Entity* left, const Entity* right) const -> bool
	{
		return left->Name() == right->Name();
	}

	auto EntityEqual::operator()(const std::string& left, const Entity* right) const -> bool
	{
		return left == right->Name();
	}

	auto EntityEqual::operator()(const Entity* left, const std::string& right) const -> bool
	{
		return left->Name() == right;
	}
}
