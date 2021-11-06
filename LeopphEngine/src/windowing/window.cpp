#include "Window.hpp"

#include "WindowBase.hpp"

#include <utility>


namespace leopph
{
	unsigned Window::Width()
	{
		return impl::WindowBase::Get().Width();
	}

	void Window::Width(const unsigned newWidth)
	{
		impl::WindowBase::Get().Width(newWidth);
	}

	unsigned Window::Height()
	{
		return impl::WindowBase::Get().Height();
	}

	void Window::Height(const unsigned newHeight)
	{
		impl::WindowBase::Get().Height(newHeight);
	}

	float Window::AspectRatio()
	{
		return impl::WindowBase::Get().AspectRatio();
	}

	bool Window::FullScreen()
	{
		return impl::WindowBase::Get().Fullscreen();
	}

	void Window::FullScreen(const bool newValue)
	{
		impl::WindowBase::Get().Fullscreen(newValue);
	}

	bool Window::Vsync()
	{
		return impl::WindowBase::Get().Vsync();
	}

	void Window::Vsync(const bool newValue)
	{
		impl::WindowBase::Get().Vsync(newValue);
	}

	std::string_view Window::Title()
	{
		return impl::WindowBase::Get().Title();
	}

	void Window::Title(std::string newTitle)
	{
		impl::WindowBase::Get().Title(std::move(newTitle));
	}

	float Window::RenderMultiplier()
	{
		return impl::WindowBase::Get().RenderMultiplier();
	}

	void Window::RenderMultiplier(const float newMult)
	{
		impl::WindowBase::Get().RenderMultiplier(newMult);
	}

}