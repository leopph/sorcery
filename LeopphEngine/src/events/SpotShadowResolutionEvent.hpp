#pragma once

#include "Event.hpp"

#include <cstddef>



namespace leopph::impl
{
	class SpotShadowResolutionEvent final : public Event
	{
		public:
			explicit SpotShadowResolutionEvent(std::size_t resolution);

			const std::size_t Resolution;
	};
}
