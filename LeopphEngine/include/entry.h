#pragma once

#include "window.h"
#include "gl.h"
#include "object.h"
#include "renderer.h"
#include "timekeeping.h"

namespace leopph
{
	// Sets the initial state of the game
	extern void Init();
}


int main(int argc, char** argv)
{
	leopph::implementation::Window& window{ leopph::implementation::Window::CreateWindow(1280, 720, "LeopphEngine Application", false) };

	if (!leopph::implementation::InitGL())
	{
		leopph::implementation::TerminateGL();
		return -1;
	}

	leopph::Init();

	while (!window.ShouldClose())
	{
		for (auto& object : leopph::Object::Instances())
			for (auto& behavior : object->Behaviors())
				behavior->operator()();

		window.Clear();

		leopph::implementation::Renderer::Instance().Render();

		leopph::Time::OnFrameComplete();

		window.SwapBuffers();
		window.PollEvents();
	}

	leopph::implementation::TerminateGL();

	return 0;
}