#pragma once

#include "Event.hpp"

#include <cstddef>


namespace leopph::internal
{
	class SpotShadowResolutionEvent final : public Event
	{
		public:
			explicit SpotShadowResolutionEvent(std::size_t resolution);

			const std::size_t Resolution;
	};
}
