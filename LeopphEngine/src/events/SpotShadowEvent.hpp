#pragma once

#include "Event.hpp"

#include <cstddef>


namespace leopph::internal
{
	// An Event representing a change in SpotLight shadow map resolution.
	struct SpotShadowEvent final : Event
	{
		explicit SpotShadowEvent(std::size_t res);
		std::size_t Resolution;
	};
}
