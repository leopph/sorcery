#include "Exit.hpp"

#include "InternalContext.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph
{
	void Exit()
	{
		internal::GetWindowImpl()->ShouldClose(true);
	}
}
