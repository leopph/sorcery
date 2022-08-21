#include "Exit.hpp"

#include "Context.hpp"
#include "Window.hpp"


namespace leopph
{
	void exit()
	{
		get_main_window().set_should_close(true);
	}
}
