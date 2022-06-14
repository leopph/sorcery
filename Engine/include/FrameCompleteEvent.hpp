#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// An Event representing the current frame being completed.
	struct FrameCompleteEvent final : Event
	{};
}
