#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// An Event representing a change in DirectionalLight shadow map resolution.
	struct DirShadowResEvent final : Event{};
}
