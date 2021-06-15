#include "launch.h"
#include "../windowing/window.h"
#include "../rendering/opengl/gl.h"
#include "../hierarchy/object.h"
#include "../rendering/renderer.h"
#include "../timing/timekeeping.h"
#include "../input/inputhandler.h"

namespace leopph::impl
{
	int Launch(decltype(AppStart) appStart)
	{
		auto& window{ leopph::impl::Window::Get(1280, 720, "LeopphEngine Application", false) };

		if (!leopph::impl::InitGL())
		{
			leopph::impl::TerminateGL();
			return -1;
		}

		appStart();

		leopph::Time::Init();

		while (!window.ShouldClose())
		{
			leopph::impl::InputHandler::UpdateReleasedKeys();
			window.PollEvents();
			leopph::Behavior::UpdateAll();
			window.Clear();
			leopph::impl::Renderer::Instance().Render();
			leopph::Time::OnFrameComplete();
			window.SwapBuffers();
		}

		leopph::impl::Window::Destroy();
		leopph::impl::TerminateGL();
		return 0;
	}
}