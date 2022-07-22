#pragma once

#include "Event.hpp"
#include "Vector.hpp"


namespace leopph::internal
{
	// An Event representing mouse movement.
	struct MouseEvent final : Event
	{
		explicit MouseEvent(Vector2 pos);
		Vector2 const Position;
	};
}
