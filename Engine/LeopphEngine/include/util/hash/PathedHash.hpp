#pragma once

#include "util/concepts/Pathed.hpp"

#include <cstddef>


namespace leopph::internal
{
	template<Pathed T>
	struct PathedHash
	{
		using is_transparent = void;

		// For references.
		auto operator()(const T& pathed) const -> std::size_t
		{
			return hash_value(pathed.Path());
		}

		// For pointers.
		auto operator()(const T* const model) const -> std::size_t
		{
			return hash_value(model->Path());
		}

		auto operator()(const std::filesystem::path& path) const -> std::size_t
		{
			return hash_value(path);
		}
	};
}
