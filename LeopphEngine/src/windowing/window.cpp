#include "Window.hpp"

#include "WindowBase.hpp"

#include <utility>


namespace leopph
{
	unsigned Window::Width()
	{
		return internal::WindowBase::Get().Width();
	}

	void Window::Width(const unsigned newWidth)
	{
		internal::WindowBase::Get().Width(newWidth);
	}

	unsigned Window::Height()
	{
		return internal::WindowBase::Get().Height();
	}

	void Window::Height(const unsigned newHeight)
	{
		internal::WindowBase::Get().Height(newHeight);
	}

	float Window::AspectRatio()
	{
		return internal::WindowBase::Get().AspectRatio();
	}

	bool Window::FullScreen()
	{
		return internal::WindowBase::Get().Fullscreen();
	}

	void Window::FullScreen(const bool newValue)
	{
		internal::WindowBase::Get().Fullscreen(newValue);
	}

	bool Window::Vsync()
	{
		return internal::WindowBase::Get().Vsync();
	}

	void Window::Vsync(const bool newValue)
	{
		internal::WindowBase::Get().Vsync(newValue);
	}

	std::string_view Window::Title()
	{
		return internal::WindowBase::Get().Title();
	}

	void Window::Title(std::string newTitle)
	{
		internal::WindowBase::Get().Title(std::move(newTitle));
	}

	float Window::RenderMultiplier()
	{
		return internal::WindowBase::Get().RenderMultiplier();
	}

	void Window::RenderMultiplier(const float newMult)
	{
		internal::WindowBase::Get().RenderMultiplier(newMult);
	}

}