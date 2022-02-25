#include "Window.hpp"


namespace leopph
{
	auto Window::Instance() -> Window*
	{
		return s_Instance;
	}


	auto Window::AspectRatio() const -> float
	{
		return static_cast<float>(Width()) / static_cast<float>(Height());
	}


	Window::Window()
	{
		s_Instance = this;
	}


	Window::~Window()
	{
		s_Instance = nullptr;
	}


	Window* Window::s_Instance{nullptr};
}
