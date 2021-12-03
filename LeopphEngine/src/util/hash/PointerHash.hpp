#pragma once

#include <cstddef>
#include <functional>
#include <memory>


namespace leopph::impl
{
	class PointerHash
	{
	public:
		using is_transparent = void;

		template<class Ptr>
		std::size_t operator()(const Ptr& ptr) const
		{
			return std::hash<Ptr>{}(ptr);
		}
	};
}