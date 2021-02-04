#pragma once

#include "window.h"
#include "gl.h"
#include "object.h"

namespace leopph
{
	// Sets the initial state of the game
	extern void Init();
}


int main(int argc, char** argv)
{
	leopph::Window& window{ leopph::Window::CreateWindow(1280, 720, "LeopphEngine Application", false) };

	if (!leopph::InitGL())
	{
		leopph::TerminateGL();
		return -1;
	}

	leopph::Init();

	while (!window.ShouldClose())
	{
		for (auto& object : leopph::Object::Instances())
			for (auto& behavior : object->Behaviors())
				behavior->operator()();

		window.Clear();
		window.PollEvents();
		window.SwapBuffers();
	}

	leopph::TerminateGL();

	return 0;
}