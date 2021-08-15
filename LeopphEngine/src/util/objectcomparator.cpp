#include "objectcomparator.h"

namespace leopph::impl
{
	bool ObjectComparator::operator()(const Object* left, const Object* right) const
	{
		return left->name < right->name;
	}

	bool ObjectComparator::operator()(const Object* left, const std::string& right) const
	{
		return left->name < right;
	}

	bool ObjectComparator::operator()(const std::string& left, const Object* right) const
	{
		return left < right->name;
	}
}