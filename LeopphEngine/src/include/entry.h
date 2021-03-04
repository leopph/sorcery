#pragma once


#include "window.h"
#include "gl.h"
#include "object.h"
#include "renderer.h"
#include "timekeeping.h"
#include "input.h"
#include "instancedata.h"



namespace leopph
{
	void Init();
}




int main()
{
	leopph::implementation::Window& window{ leopph::implementation::Window::Get(1280, 720, "LeopphEngine Application", false) };


	if (!leopph::implementation::InitGL())
	{
		leopph::implementation::TerminateGL();
		return -1;
	}


	leopph::Init();


	while (!window.ShouldClose())
	{
		leopph::Input::UpdateReleasedKeys();
		window.PollEvents();
		leopph::Object::UpdateAll();
		window.Clear();
		leopph::implementation::Renderer::Instance().Render();
		leopph::Time::OnFrameComplete();
		window.SwapBuffers();
	}


	leopph::implementation::Window::Destroy();
	leopph::implementation::TerminateGL();

	return 0;
}