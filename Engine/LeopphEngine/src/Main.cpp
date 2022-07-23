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

#include "EventManager.hpp"
#include "FrameCompleteEvent.hpp"
#include "InternalContext.hpp"
#include "Logger.hpp"
#include "SettingsImpl.hpp"
#include "data/DataManager.hpp"
#include "rendering/Renderer.hpp"
#include "windowing/WindowImpl.hpp"


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

		auto const settings = std::make_unique<SettingsImpl>();
		SetSettingsImpl(settings.get());

		auto const window = WindowImpl::Create();
		SetWindowImpl(window.get());

		auto const renderer = Renderer::Create();
		SetRenderer(renderer.get());

		auto const dataManager = std::make_unique<DataManager>();
		SetDataManager(dataManager.get());

		initFunc();

		while (!window->ShouldClose())
		{
			window->PollEvents();

			for (auto const& behavior : dataManager->ActiveBehaviors())
			{
				behavior->OnFrameUpdate();
			}

			window->Clear();
			renderer->Render();
			window->SwapBuffers();
			EventManager::Instance().Send<FrameCompleteEvent>();
		}

		return 0;
	}
}
