#include "Main.hpp"

#ifdef _DEBUG
// Order here is important:
// 1. define
// 2. stdlib
// 3. crtdbg

// ReSharper disable All
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
// ReSharper restore All
#endif

#include "Context.hpp"
#include "EventManager.hpp"
#include "FrameCompleteEvent.hpp"
#include "Logger.hpp"
//#include "SettingsImpl.hpp"
#include "Window.hpp"
//#include "data/DataManager.hpp"
#include "Behavior.hpp"
#include "Timer.hpp"
#include "rendering/Renderer.hpp"
#include "Scene/SceneManager.hpp"


namespace leopph::internal
{
	int main(decltype(init) initFunc)
	{
		#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		Logger::get_instance().set_level(Logger::Level::Trace);
		#else
		Logger::get_instance().set_level(Logger::Level::Info);
		#endif

		//auto const settings = std::make_unique<SettingsImpl>();
		//SetSettingsImpl(settings.get());

		auto const window = std::make_unique<Window>();
		set_main_window(window.get());

		auto const renderer = std::make_unique<Renderer>();
		set_renderer(renderer.get());

		auto const sceneManager = std::make_unique<SceneManager>();
		set_scene_manager(sceneManager.get());

		auto const timer = std::make_unique<Timer>();
		set_frame_timer(timer.get());

		initFunc();

		timer->init();

		while (!window->should_close())
		{
			window->poll_events();
			Component::init_all();
			Behavior::update_all();
			renderer->render();
			window->swap_buffers();
			EventManager::get_instance().send<FrameCompleteEvent>();
			timer->tick();
		}

		return 0;
	}
}
