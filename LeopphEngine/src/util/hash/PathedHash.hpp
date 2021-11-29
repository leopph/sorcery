#pragma once

#include "../../rendering/geometry/ModelImpl.hpp"
#include "../concepts/Pathed.hpp"

#include <cstddef>


namespace leopph::impl
{
	template<Pathed T>
	struct PathedHash
	{
		using is_transparent = void;

		std::size_t operator()(const T& model) const
		{
			return hash_value(model.Path);
		}

		std::size_t operator()(const std::filesystem::path& path) const
		{
			return hash_value(path);
		}
	};
}
