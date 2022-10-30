#include <Components.hpp>
#include <ManagedRuntime.hpp>
#include <Platform.hpp>
#include <RenderCore.hpp>
#include <Time.hpp>
#include <Entity.hpp>


int main()
{
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

		renderer->present();

		leopph::measure_time();
	}

	leopph::gEntities.clear();
	leopph::cleanup_managed_runtime();
	leopph::platform::cleanup_platform_support();
}
