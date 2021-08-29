#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "launch.h"

#include "../input/inputhandler.h"
#include "../instances/DataManager.hpp"
#include "../rendering/DeferredRenderer.hpp"
#include "../rendering/ForwardRenderer.hpp"
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
		Logger::Instance().Debug("Window created.");

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
			Logger::Instance().Debug("Renderer initialized");

			Timer::Init();
			Logger::Instance().Debug("Timer initialized.");

			Logger::Instance().Debug("Starting render loop.");
			while (!window.ShouldClose())
			{
				InputHandler::UpdateReleasedKeys();
				window.PollEvents();

				for (const auto& x : DataManager::Behaviors())
					x->OnFrameUpdate();

				window.Clear();
				renderer.Render();
				Timer::OnFrameComplete();
				window.SwapBuffers();
			}
			Logger::Instance().Debug("Render loop finished.");
		}
		Logger::Instance().Debug("Renderer destroyed");

		DataManager::DestroyAllObjects();
		Logger::Instance().Debug("All objects destroyed.");

		Window::Destroy();
		Logger::Instance().Debug("Window destroyed.");

		return 0;
	}
}
