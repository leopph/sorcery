#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "launch.h"

#include "../windowing/window.h"
#include "../rendering/opengl/gl.h"
#include "../rendering/renderer.h"
#include "../timing/timer.h"
#include "../input/inputhandler.h"
#include "../instances/instanceholder.h"

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
			Renderer renderer;
			Logger::Instance().Debug("Renderer initialized");

			Timer::Init();
			Logger::Instance().Debug("Timer initialized.");

			Logger::Instance().Debug("Starting render loop.");
			while (!window.ShouldClose())
			{
				InputHandler::UpdateReleasedKeys();
				window.PollEvents();

				for (const auto& x : InstanceHolder::Behaviors())
					x->OnFrameUpdate();

				window.Clear();
				renderer.Render();
				Timer::OnFrameComplete();
				window.SwapBuffers();
			}
			Logger::Instance().Debug("Render loop finished.");
		}
		Logger::Instance().Debug("Renderer destroyed");

		InstanceHolder::DestroyAll();
		Logger::Instance().Debug("All objects destroyed.");

		Window::Destroy();
		Logger::Instance().Debug("Window destroyed.");

		return 0;
	}
}
