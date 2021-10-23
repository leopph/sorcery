#pragma once

#include "Event.hpp"

#include <cstddef>



namespace leopph::impl
{
	class PointShadowMapChangedEvent final : public Event
	{
		public:
			explicit PointShadowMapChangedEvent(std::size_t resolution);

			const std::size_t Resolution;
	};
}
