#pragma once

#include "../concepts/Pathed.hpp"


namespace leopph::internal
{
	template<Pathed T>
	struct PathedEqual
	{
		using is_transparent = void;

		// For references.

		auto operator()(const T& left, const T& right) const -> bool
		{
			return left.Path() == right.Path();
		}

		auto operator()(const std::filesystem::path& left, const T& right) const -> bool
		{
			return left == right.Path();
		}

		auto operator()(const T& left, const std::filesystem::path& right) const -> bool
		{
			return left.Path() == right;
		}

		// For pointers.

		auto operator()(const T* const left, const T* const right) const -> bool
		{
			return left->Path() == right->Path();
		}

		auto operator()(const std::filesystem::path& left, const T* const right) const -> bool
		{
			return left == right->Path();
		}

		auto operator()(const T* const left, const std::filesystem::path& right) const -> bool
		{
			return left->Path() == right;
		}
	};
}
