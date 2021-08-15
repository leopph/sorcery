#pragma once

#include "../hierarchy/Object.hpp"

#include <string>

namespace leopph::impl
{
	struct ObjectComparator
	{
		using is_transparent = void;
		bool operator()(const Object* left, const Object* right) const;
		bool operator()(const Object* left, const std::string& right) const;
		bool operator()(const std::string& left, const Object* right) const;
	};
}