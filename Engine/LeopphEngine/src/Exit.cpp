#include "Exit.hpp"

#include "InternalContext.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph
{
	auto Exit() -> void
	{
		internal::GetWindowImpl()->ShouldClose(true);
	}
}
