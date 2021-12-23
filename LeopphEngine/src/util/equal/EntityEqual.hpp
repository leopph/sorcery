#pragma once

#include "../../entity/Entity.hpp"

#include <string>


namespace leopph::internal
{
	struct EntityEqual
	{
		using is_transparent = void;

		bool operator()(const Entity* left, const Entity* right) const;
		bool operator()(const std::string& left, const Entity* right) const;
		bool operator()(const Entity* left, const std::string& right) const;
	};
}
