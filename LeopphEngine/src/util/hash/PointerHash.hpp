#pragma once

#include <cstddef>
#include <functional>
#include <memory>


namespace leopph::internal
{
	class PointerHash
	{
		public:
			using is_transparent = void;

			template<class Ptr>
			auto operator()(const Ptr& ptr) const -> std::size_t
			{
				return std::hash<Ptr>{}(ptr);
			}
	};
}
