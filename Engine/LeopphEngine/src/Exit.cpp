#include "Exit.hpp"

#include "windowing/WindowImpl.hpp"

namespace leopph
{
	auto Exit() -> void
	{
		static_cast<internal::WindowImpl*>(Window::Instance())->ShouldClose(true);
	}

}