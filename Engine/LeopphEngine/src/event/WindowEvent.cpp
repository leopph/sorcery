#include "WindowEvent.hpp"


namespace leopph::internal
{
	WindowEvent::WindowEvent(unsigned const width, unsigned const height, float const renderMult, bool const fullscreen, bool const vsync) :
		Width{width},
		Height{height},
		RenderMultiplier{renderMult},
		Fullscreen{fullscreen},
		Vsync{vsync}
	{}
}
