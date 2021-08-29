#pragma once

#include "../math/Vector.hpp"
#include "Event.hpp"


namespace leopph::impl
{
	struct MouseEvent final : Event
	{
		const Vector2 position;

		explicit MouseEvent(const Vector2& pos);
	};
}