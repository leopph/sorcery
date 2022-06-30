#include "Window.hpp"


namespace leopph
{
	auto Window::AspectRatio() const -> float
	{
		return static_cast<float>(Width()) / static_cast<float>(Height());
	}
}
