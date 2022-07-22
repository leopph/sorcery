#include "Window.hpp"


namespace leopph
{
	float Window::AspectRatio() const
	{
		return static_cast<float>(Width()) / static_cast<float>(Height());
	}
}
