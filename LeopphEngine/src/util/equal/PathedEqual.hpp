#pragma once

#include "../concepts/Pathed.hpp"


namespace leopph::internal
{
	template<Pathed T>
	struct PathedEqual 
	{
		using is_transparent = void;

		// For references.

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

		// For pointers.


		bool operator()(const T* const left, const T* const right) const
		{
			return left->Path == right->Path;
		}

		bool operator()(const std::filesystem::path& left, const T* const right) const
		{
			return left == right->Path;
		}

		bool operator()(const T* const left, const std::filesystem::path& right) const
		{
			return left->Path == right;
		}
	};
}
