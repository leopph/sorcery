#include "ObjectLess.hpp"

namespace leopph::impl
{
	bool ObjectLess::operator()(const Object* left, const Object* right) const
	{
		return left->name < right->name;
	}

	bool ObjectLess::operator()(const Object* left, const std::string& right) const
	{
		return left->name < right;
	}

	bool ObjectLess::operator()(const std::string& left, const Object* right) const
	{
		return left < right->name;
	}
}