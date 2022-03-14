#pragma once

#include "Event.hpp"
#include "../math/Vector.hpp"


namespace leopph::internal
{
	// An Event representing mouse movement.
	struct MouseEvent final : Event
	{
		explicit MouseEvent(Vector2 pos);
		const Vector2 Position;
	};
}
