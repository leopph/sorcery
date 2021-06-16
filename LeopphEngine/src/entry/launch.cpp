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

namespace leopph::impl
{
	int Launch(decltype(AppStart) appStart)
	{
#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
		
		auto& window{ Window::Get(1280, 720, "LeopphEngine Application", false) };

		if (!InitGL())
		{
			Window::Destroy();
			return -1;
		}

		appStart();

		Timer::Init();

		while (!window.ShouldClose())
		{
			InputHandler::UpdateReleasedKeys();
			window.PollEvents();

			for (const auto& x : InstanceHolder::Behaviors())
				x->OnFrameUpdate();
			
			window.Clear();
			Renderer::Instance().Render();
			Timer::OnFrameComplete();
			window.SwapBuffers();
		}

		InstanceHolder::DestroyAll();
		Window::Destroy();
		return 0;
	}
}
