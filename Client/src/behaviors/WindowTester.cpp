#include "WindowTester.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Window;


WindowTester::WindowTester() :
	m_Window{Window::Instance()}
{}


auto WindowTester::OnFrameUpdate() -> void
{
	if (Input::GetKeyDown(KeyCode::F))
	{
		m_Window->Fullscreen(!m_Window->Fullscreen());
	}

	if (Input::GetKeyDown(KeyCode::V))
	{
		m_Window->Vsync(!m_Window->Vsync());
	}

	if (Input::GetKeyDown(KeyCode::One))
	{
		m_Window->Width(1920);
		m_Window->Height(1080);
	}

	if (Input::GetKeyDown(KeyCode::Two))
	{
		m_Window->Width(1600);
		m_Window->Height(900);
	}

	if (Input::GetKeyDown(KeyCode::Three))
	{
		m_Window->Width(1280);
		m_Window->Height(720);
	}

	if (Input::GetKeyDown(KeyCode::Four))
	{
		m_Window->Width(1024);
		m_Window->Height(768);
	}

	if (Input::GetKeyDown(KeyCode::Five))
	{
		m_Window->Width(800);
		m_Window->Height(600);
	}

	if (Input::GetKeyDown(KeyCode::Six))
	{
		m_Window->Width(640);
		m_Window->Height(480);
	}

	if (Input::GetKeyDown(KeyCode::Q))
	{
		if (m_Window->RenderMultiplier() > 0.1f)
		{
			m_Window->RenderMultiplier(m_Window->RenderMultiplier() - 0.1f);
		}
	}

	if (Input::GetKeyDown(KeyCode::E))
	{
		m_Window->RenderMultiplier(m_Window->RenderMultiplier() + 0.1f);
	}
}
