#pragma once

#include "Event.hpp"

#include <cstddef>



namespace leopph::impl
{
	class SpotShadowMapResChangedEvent final : public Event
	{
		public:
			explicit SpotShadowMapResChangedEvent(std::size_t resolution);

			const std::size_t Resolution;
	};
}
