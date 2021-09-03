#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "launch.h"

#include "../events/EventManager.hpp"
#include "../events/FrameBeginsEvent.hpp"
#include "../data/DataManager.hpp"
#include "../rendering/renderers/DeferredRenderer.hpp"
#include "../rendering/renderers/ForwardRenderer.hpp"
#include "../rendering/opengl/gl.h"
#include "../timing/timer.h"
#include "../windowing/window.h"

#include "../util/logger.h"

namespace leopph::impl
{
	int Launch(decltype(AppStart) appStart)
	{
#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		Logger::Instance().CurrentLevel(Logger::Level::DEBUG);
#endif
		
		auto& window{ Window::Get(1280, 720, "LeopphEngine Application", false) };

		if (!InitGL())
		{
			Window::Destroy();
			Logger::Instance().Critical("OpenGL could not be initialized.");
			return -1;
		}
		Logger::Instance().Debug("OpenGL initialized.");

		appStart();
		Logger::Instance().Debug("App initialized.");

		{
			ForwardRenderer renderer;

			Timer::Init();
			Logger::Instance().Debug("Timer initialized.");

			while (!window.ShouldClose())
			{
				EventManager::Instance().Send<FrameBeginsEvent>();
				window.PollEvents();

				for (const auto& x : DataManager::Behaviors())
					x->OnFrameUpdate();

				window.Clear();
				renderer.Render();
				Timer::OnFrameComplete();
				window.SwapBuffers();
			}
		}

		DataManager::Clear();
		Window::Destroy();
		return 0;
	}
}
