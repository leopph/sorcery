#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// An Event representing a change in PointLight shadow map resolution.
	struct PointShadowResEvent final : Event{};
}
