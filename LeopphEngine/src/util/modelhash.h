#pragma once

#include "../rendering/model.h"

namespace std
{
	template<>
	struct hash<leopph::Model>
	{
		std::size_t operator()(const leopph::Model& model) const noexcept
		{
			return hash_value(model.Path());
		}
	};
}