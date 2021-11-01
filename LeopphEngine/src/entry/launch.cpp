#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "launch.h"

#include "../data/DataManager.hpp"
#include "../events/EventManager.hpp"
#include "../events/FrameEndedEvent.hpp"
#include "../rendering/opengl/gl.h"
#include "../rendering/renderers/Renderer.hpp"
#include "../timing/timer.h"
#include "../util/logger.h"
#include "../windowing/WindowBase.hpp"



namespace leopph::impl
{
	int Launch(decltype(AppStart) appStart)
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

		appStart();

		Logger::Instance().Debug("App initialized.");

		{
			const auto renderer{Renderer::Create()};

			Timer::Init();

			Logger::Instance().Debug("Timer initialized.");

			while (!window.ShouldClose())
			{
				window.PollEvents();

				for (const auto& x : DataManager::Behaviors())
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

		DataManager::Clear();

		WindowBase::Destroy();

		return 0;
	}
}
