#ifdef _DEBUG
// Order here is important:
// 1. define
// 2. stdlib
// 3. crtdbg
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "Main.hpp"

#include "../data/DataManager.hpp"
#include "../events/FrameCompleteEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../rendering/renderers/Renderer.hpp"
#include "../util/Logger.hpp"
#include "../windowing/WindowImpl.hpp"


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

		DataManager::Instance().Clear();
		return 0;
	}
}
