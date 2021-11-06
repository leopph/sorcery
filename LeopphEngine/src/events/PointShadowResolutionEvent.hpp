#pragma once

#include "Event.hpp"

#include <cstddef>



namespace leopph::impl
{
	class PointShadowResolutionEvent final : public Event
	{
		public:
			explicit PointShadowResolutionEvent(std::size_t resolution);

			const std::size_t Resolution;
	};
}
