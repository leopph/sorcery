#pragma once

#include "../../rendering/geometry/ModelImpl.hpp"
#include "../concepts/Pathed.hpp"


namespace leopph::impl
{
	template<Pathed T>
	struct PathedEqual 
	{
		using is_transparent = void;

		bool operator()(const T& left, const T& right) const
		{
			return left.Path == right.Path;
		}

		bool operator()(const std::filesystem::path& left, const T& right) const
		{
			return left == right.Path;
		}

		bool operator()(const T& left, const std::filesystem::path& right) const
		{
			return left.Path == right;
		}
	};
}
