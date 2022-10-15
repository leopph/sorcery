#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <Behavior.hpp>
#include <Managed.hpp>
#include <Platform.hpp>
#include <RenderCore.hpp>
#include <Time.hpp>


int main()
{
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (!leopph::platform::init_platform_support())
	{
		return 1;
	}

	auto const renderer = leopph::RenderCore::create();

	if (!renderer)
	{
		return 2;
	}

	if (!leopph::initialize_managed_runtime())
	{
		return 3;
	}

	leopph::init_time();

	while (!leopph::platform::should_window_close())
	{
		if (!leopph::platform::process_platform_events())
		{
			return 4;
		}

		leopph::init_behaviors();
		leopph::tick_behaviors();
		leopph::tack_behaviors();

		if (!renderer->render())
		{
			return 5;
		}

		leopph::measure_time();
	}

	leopph::cleanup_managed_runtime();
	leopph::platform::cleanup_platform_support();
}
