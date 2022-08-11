#include "WindowEvent.hpp"


namespace leopph
{
	internal::WindowEvent::WindowEvent(u32 const width, u32 const height, bool const fullscreen, bool const vsync) :
		width{width},
		height{height},
		fullscreen{fullscreen},
		vsync{vsync}
	{ }
}
