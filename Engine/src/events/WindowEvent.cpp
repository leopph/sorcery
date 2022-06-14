#include "WindowEvent.hpp"


namespace leopph::internal
{
	WindowEvent::WindowEvent(const unsigned width, const unsigned height, const float renderMult, const bool fullscreen, const bool vsync) :
		Width{width},
		Height{height},
		RenderMultiplier{renderMult},
		Fullscreen{fullscreen},
		Vsync{vsync}
	{}
}
