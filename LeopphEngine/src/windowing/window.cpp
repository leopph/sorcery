#include "Window.hpp"

#include "WindowBase.hpp"

#include <utility>


namespace leopph
{
	auto Window::Width() -> unsigned
	{
		return internal::WindowBase::Get().Width();
	}

	auto Window::Width(const unsigned newWidth) -> void
	{
		internal::WindowBase::Get().Width(newWidth);
	}

	auto Window::Height() -> unsigned
	{
		return internal::WindowBase::Get().Height();
	}

	auto Window::Height(const unsigned newHeight) -> void
	{
		internal::WindowBase::Get().Height(newHeight);
	}

	auto Window::AspectRatio() -> float
	{
		return internal::WindowBase::Get().AspectRatio();
	}

	auto Window::FullScreen() -> bool
	{
		return internal::WindowBase::Get().Fullscreen();
	}

	auto Window::FullScreen(const bool newValue) -> void
	{
		internal::WindowBase::Get().Fullscreen(newValue);
	}

	auto Window::Vsync() -> bool
	{
		return internal::WindowBase::Get().Vsync();
	}

	auto Window::Vsync(const bool newValue) -> void
	{
		internal::WindowBase::Get().Vsync(newValue);
	}

	auto Window::Title() -> std::string_view
	{
		return internal::WindowBase::Get().Title();
	}

	auto Window::Title(std::string newTitle) -> void
	{
		internal::WindowBase::Get().Title(std::move(newTitle));
	}

	auto Window::RenderMultiplier() -> float
	{
		return internal::WindowBase::Get().RenderMultiplier();
	}

	auto Window::RenderMultiplier(const float newMult) -> void
	{
		internal::WindowBase::Get().RenderMultiplier(newMult);
	}
}
