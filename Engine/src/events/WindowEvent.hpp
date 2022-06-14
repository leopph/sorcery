#pragma once

#include "Event.hpp"


namespace leopph::internal
{
	// An Event representing a change in window configuration.
	struct WindowEvent final : Event
	{
		WindowEvent(unsigned width, unsigned height, float renderMult, bool fullscreen, bool vsync);
		unsigned Width;
		unsigned Height;
		float RenderMultiplier;
		bool Fullscreen;
		bool Vsync;
	};
}
