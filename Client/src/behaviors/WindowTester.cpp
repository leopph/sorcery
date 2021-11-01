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
}
