#pragma once

#include "../concepts/Pathed.hpp"

#include <cstddef>


namespace leopph::internal
{
	template<Pathed T>
	struct PathedHash
	{
		using is_transparent = void;

		// For references.
		std::size_t operator()(const T& pathed) const
		{
			return hash_value(pathed.Path);
		}

		// For pointers.
		std::size_t operator()(const T* const model) const
		{
			return hash_value(model->Path);
		}

		std::size_t operator()(const std::filesystem::path& path) const
		{
			return hash_value(path);
		}
	};
}
