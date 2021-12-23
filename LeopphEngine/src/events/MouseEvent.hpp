#pragma once

#include "Event.hpp"
#include "../math/Vector.hpp"


namespace leopph::internal
{
	struct MouseEvent final : Event
	{
		const Vector2 position;

		explicit MouseEvent(const Vector2& pos);
	};
}
