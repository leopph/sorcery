#pragma once

#include <memory>


namespace leopph::impl
{
	class PointerEqual
	{
	public:
		using is_transparent = void;

		bool operator()(const auto& left, const auto& right) const
		{
			return std::to_address(left) == std::to_address(right);
		}
	};
};