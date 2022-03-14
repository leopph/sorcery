#include "WindowEvent.hpp"


namespace leopph::internal
{
	WindowEvent::WindowEvent(const Vector<unsigned, 2>& newScreenRes, const float newResMult, const bool vsync, const bool fullscreen) :
		Resolution{newScreenRes},
		RenderMultiplier{newResMult},
		Vsync{vsync},
		Fullscreen{fullscreen}
	{}
}
