#pragma once

#include "Event.hpp"
#include "Types.hpp"


namespace leopph::internal
{
	// An Event representing a change in window configuration.
	struct WindowEvent final : Event
	{
		WindowEvent(u32 width, u32 height, bool fullscreen, bool vsync);

		u32 width;
		u32 height;
		bool fullscreen;
		bool vsync;
	};
}
