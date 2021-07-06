#include "objectcomparator.h"

namespace leopph::impl
{
	bool ObjectComparator::operator()(const Object* left, const Object* right) const
	{
		return left->Name() < right->Name();
	}

	bool ObjectComparator::operator()(const Object* left, const std::string& right) const
	{
		return left->Name() < right;
	}

	bool ObjectComparator::operator()(const std::string& left, const Object* right) const
	{
		return left < right->Name();
	}
}