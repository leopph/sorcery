#include "Main.hpp"

#ifdef _DEBUG
// Order here is important:
// 1. define
// 2. stdlib
// 3. crtdbg

// ReSharper disable All
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
// ReSharper restore All
#endif

#include "EventManager.hpp"
#include "FrameCompleteEvent.hpp"
#include "Logger.hpp"
#include "data/DataManager.hpp"
#include "rendering/renderers/Renderer.hpp"
#include "threading/JobSystem.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph::internal
{
	auto Main(decltype(Init) initFunc) -> int
	{
		#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		Logger::Instance().CurrentLevel(Logger::Level::Trace);
		#else
		Logger::Instance().CurrentLevel(Logger::Level::Info);
		#endif

		auto const window{WindowImpl::Create()};
		auto const renderer{Renderer::Create()};

		initFunc();

		while (!window->ShouldClose())
		{
			window->PollEvents();

			for (auto const& behavior : DataManager::Instance().ActiveBehaviors())
			{
				behavior->OnFrameUpdate();
			}

			window->Clear();
			renderer->Render();
			window->SwapBuffers();
			EventManager::Instance().Send<FrameCompleteEvent>();
		}

		ShutDownWorkers();

		DataManager::Instance().Clear();
		return 0;
	}
}
