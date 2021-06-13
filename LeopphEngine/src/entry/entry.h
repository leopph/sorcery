#pragma once


#include "../windowing/window.h"
#include "../rendering/opengl/gl.h"
#include "../hierarchy/object.h"
#include "../rendering/renderer.h"
#include "../timing/timekeeping.h"
#include "../input/input.h"
#include "../instances/instancedata.h"



namespace leopph
{
	void AppStart();
}




int main()
{
	auto& window{ leopph::impl::Window::Get(1280, 720, "LeopphEngine Application", false) };

	if (!leopph::impl::InitGL())
	{
		leopph::impl::TerminateGL();
		return -1;
	}

	leopph::AppStart();

	leopph::Time::Init();

	while (!window.ShouldClose())
	{
		leopph::Input::UpdateReleasedKeys();
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