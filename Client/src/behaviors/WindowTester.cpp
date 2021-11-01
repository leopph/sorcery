#include "WindowTester.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Window;


void WindowTester::OnFrameUpdate()
{
	if (Input::GetKeyDown(KeyCode::F))
	{
		Window::FullScreen(!Window::FullScreen());
	}

	if (Input::GetKeyDown(KeyCode::V))
	{
		Window::Vsync(!Window::Vsync());
	}

	if (Input::GetKeyDown(KeyCode::One))
	{
		Window::Width(1920);
		Window::Height(1080);
	}

	if (Input::GetKeyDown(KeyCode::Two))
	{
		Window::Width(1600);
		Window::Height(900);
	}

	if (Input::GetKeyDown(KeyCode::Three))
	{
		Window::Width(1280);
		Window::Height(720);
	}

	if (Input::GetKeyDown(KeyCode::Four))
	{
		Window::Width(1024);
		Window::Height(768);
	}

	if (Input::GetKeyDown(KeyCode::Five))
	{
		Window::Width(800);
		Window::Height(600);
	}

	if (Input::GetKeyDown(KeyCode::Six))
	{
		Window::Width(640);
		Window::Height(480);
	}
}
