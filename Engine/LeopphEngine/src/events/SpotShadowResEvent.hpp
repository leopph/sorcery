#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// An Event representing a change in SpotLight shadow map resolution.
	struct SpotShadowResEvent final : Event
	{};
}
