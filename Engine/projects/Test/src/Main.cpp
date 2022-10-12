#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <Behavior.hpp>
#include <Input.hpp>
#include <Managed.hpp>
#include <Time.hpp>
#include <Window.hpp>
#include <RenderCore.hpp>


int main()
{
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	auto const window = leopph::Window::Create();

	if (!window)
	{
		return -1;
	}

	auto const renderer = leopph::RenderCore::Create(*window);

	if (!renderer)
	{
		return -1;
	}

	leopph::initialize_managed_runtime();

	window->show();

	leopph::init_time();

	while (!window->should_close())
	{
		window->process_events();

		leopph::update_keyboard_state();

		leopph::init_behaviors();
		leopph::tick_behaviors();
		leopph::tack_behaviors();

		if (!renderer->render())
		{
			return -1;
		}

		leopph::measure_time();
	}
}
