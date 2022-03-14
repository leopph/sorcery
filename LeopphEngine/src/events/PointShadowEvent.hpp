#pragma once

#include "Event.hpp"

#include <cstddef>


namespace leopph::internal
{
	// An Event representing a change in PointLight shadow map resolution.
	struct PointShadowEvent final : Event
	{
		explicit PointShadowEvent(std::size_t res);
		std::size_t Resolution;
	};
}
