#include "WindowController.hpp"

#include "Context.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Window;


WindowController::WindowController() :
	m_Window{leopph::GetWindow()},
	m_DisplayModes
	{
		[this]
		{
			std::vector<Window::DisplayMode> displayModes;
			for (auto const& displayMode : m_Window->GetSupportedDisplayModes())
			{
				if (displayModes.empty() || displayModes.back().Width != displayMode.Width || displayModes.back().Height != displayMode.Height)
				{
					displayModes.push_back(displayMode);
				}
			}
			return displayModes;
		}()
	}
{}


void WindowController::OnFrameUpdate()
{
	if (Input::GetKeyDown(KeyCode::F))
	{
		m_Window->Fullscreen(!m_Window->Fullscreen());
	}

	if (Input::GetKeyDown(KeyCode::V))
	{
		m_Window->Vsync(!m_Window->Vsync());
	}

	if (Input::GetKeyDown(KeyCode::One) && !m_DisplayModes.empty())
	{
		m_Window->Width(m_DisplayModes[0].Width);
		m_Window->Height(m_DisplayModes[0].Height);
	}

	if (Input::GetKeyDown(KeyCode::Two) && m_DisplayModes.size() > 1)
	{
		m_Window->Width(m_DisplayModes[1].Width);
		m_Window->Height(m_DisplayModes[1].Height);
	}

	if (Input::GetKeyDown(KeyCode::Three) && m_DisplayModes.size() > 2)
	{
		m_Window->Width(m_DisplayModes[2].Width);
		m_Window->Height(m_DisplayModes[2].Height);
	}

	if (Input::GetKeyDown(KeyCode::Four) && m_DisplayModes.size() > 3)
	{
		m_Window->Width(m_DisplayModes[3].Width);
		m_Window->Height(m_DisplayModes[3].Height);
	}

	if (Input::GetKeyDown(KeyCode::Five) && m_DisplayModes.size() > 4)
	{
		m_Window->Width(m_DisplayModes[4].Width);
		m_Window->Height(m_DisplayModes[4].Height);
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
