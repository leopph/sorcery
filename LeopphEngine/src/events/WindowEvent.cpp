#include "WindowEvent.hpp"


namespace leopph::internal
{
	WindowEvent::WindowEvent(const Vector2& newScreenRes, const float newResMult, const bool vsync, const bool fullscreen) :
		Resolution{newScreenRes},
		RenderMultiplier{newResMult},
		Vsync{vsync},
		Fullscreen{fullscreen}
	{}
}
