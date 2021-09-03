#include "ObjectEqual.hpp"


namespace leopph::impl
{
	bool ObjectEqual::operator()(const Object* left, const Object* right) const
	{
		return left->name == right->name;
	}


	bool ObjectEqual::operator()(const std::string& left, const Object* right) const
	{
		return left == right->name;
	}


	bool ObjectEqual::operator()(const Object* left, const std::string& right) const
	{
		return left->name == right;
	}
}