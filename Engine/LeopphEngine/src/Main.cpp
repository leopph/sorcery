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
#include "rendering/Renderer.hpp"


namespace leopph::internal
{
	int Main(decltype(Init) initFunc)
	{
		#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		Logger::Instance().CurrentLevel(Logger::Level::Trace);
		#else
		Logger::Instance().CurrentLevel(Logger::Level::Info);
		#endif

		//auto const settings = std::make_unique<SettingsImpl>();
		//SetSettingsImpl(settings.get());

		auto const window = std::make_unique<Window>();
		set_window(window.get());

		auto const renderer = std::make_unique<Renderer>();
		set_renderer(renderer.get());

		//auto const dataManager = std::make_unique<DataManager>();
		//(dataManager.get());

		initFunc();

		while (!window->should_close())
		{
			window->poll_events();

			//for (auto const& behavior : dataManager->ActiveBehaviors())
			{
				//behavior->OnFrameUpdate();
			}

			renderer->render();
			window->swap_buffers();
			EventManager::Instance().Send<FrameCompleteEvent>();
		}

		return 0;
	}
}
