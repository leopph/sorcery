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
#include "../events/FrameEndEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../rendering/renderers/Renderer.hpp"
#include "../timing/timer.h"
#include "../util/logger.h"
#include "../windowing/WindowImpl.hpp"


namespace leopph::internal
{
	auto Main(decltype(Init) initFunc) -> int
	{
		#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		Logger::Instance().CurrentLevel(Logger::Level::DEBUG);
		#endif

		const auto window{WindowImpl::Create()};
		const auto renderer{Renderer::Create()};

		initFunc();
		Logger::Instance().Debug("App initialized.");

		Timer::Init();

		while (!window->ShouldClose())
		{
			window->PollEvents();

			for (const auto& behavior : DataManager::Instance().ActiveBehaviors())
			{
				behavior->OnFrameUpdate();
			}

			window->Clear();
			renderer->Render();
			Timer::OnFrameComplete();
			window->SwapBuffers();
			EventManager::Instance().Send<FrameEndedEvent>();
		}

		DataManager::Instance().Clear();
		return 0;
	}
}
