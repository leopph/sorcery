#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "Main.hpp"

#include "../data/DataManager.hpp"
#include "../events/FrameEndEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../rendering/opengl/InitGl.hpp"
#include "../rendering/renderers/Renderer.hpp"
#include "../timing/timer.h"
#include "../util/logger.h"
#include "../windowing/WindowBase.hpp"


namespace leopph::impl
{
	int Main(decltype(Init) initFunc)
	{
		#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		Logger::Instance().CurrentLevel(Logger::Level::DEBUG);
		#endif

		auto& window{WindowBase::Get()};

		if (!InitGL())
		{
			WindowBase::Destroy();
			Logger::Instance().Critical("OpenGL could not be initialized.");
			return -1;
		}

		Logger::Instance().Debug("OpenGL initialized.");

		{
			const auto renderer{Renderer::Create()};

			Logger::Instance().Debug("Renderer initialized.");

			Timer::Init();

			Logger::Instance().Debug("Timer initialized.");

			initFunc();

			Logger::Instance().Debug("App initialized.");

			while (!window.ShouldClose())
			{
				window.PollEvents();

				for (const auto& x : DataManager::Instance().Behaviors())
				{
					x->OnFrameUpdate();
				}

				window.Clear();

				renderer->Render();

				Timer::OnFrameComplete();

				window.SwapBuffers();

				EventManager::Instance().Send<FrameEndedEvent>();
			}
		}

		DataManager::Instance().Clear();

		Logger::Instance().Debug("Application data cleared.");

		WindowBase::Destroy();

		return 0;
	}
}
