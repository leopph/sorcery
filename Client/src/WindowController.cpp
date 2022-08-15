#include "WindowController.hpp"

#include "Context.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Window;


WindowController::WindowController() :
	m_Window{leopph::get_window()},
	m_DisplayModes
	{
		[this]
		{
			std::vector<Window::DisplayMode> displayModes;
			for (auto const& displayMode : m_Window->get_supported_display_modes())
			{
				if (displayModes.empty() || displayModes.back().width != displayMode.width || displayModes.back().height != displayMode.height)
				{
					displayModes.push_back(displayMode);
				}
			}
			return displayModes;
		}()
	}
{}


void WindowController::on_frame_update()
{
	if (Input::GetKeyDown(KeyCode::F))
	{
		m_Window->set_fullscreen(!m_Window->is_fullscreen());
	}

	if (Input::GetKeyDown(KeyCode::V))
	{
		m_Window->set_vsync(!m_Window->get_vsync());
	}

	if (Input::GetKeyDown(KeyCode::One) && !m_DisplayModes.empty())
	{
		m_Window->set_width(m_DisplayModes[0].width);
		m_Window->set_height(m_DisplayModes[0].height);
	}

	if (Input::GetKeyDown(KeyCode::Two) && m_DisplayModes.size() > 1)
	{
		m_Window->set_width(m_DisplayModes[1].width);
		m_Window->set_height(m_DisplayModes[1].height);
	}

	if (Input::GetKeyDown(KeyCode::Three) && m_DisplayModes.size() > 2)
	{
		m_Window->set_width(m_DisplayModes[2].width);
		m_Window->set_height(m_DisplayModes[2].height);
	}

	if (Input::GetKeyDown(KeyCode::Four) && m_DisplayModes.size() > 3)
	{
		m_Window->set_width(m_DisplayModes[3].width);
		m_Window->set_height(m_DisplayModes[3].height);
	}

	if (Input::GetKeyDown(KeyCode::Five) && m_DisplayModes.size() > 4)
	{
		m_Window->set_width(m_DisplayModes[4].width);
		m_Window->set_height(m_DisplayModes[4].height);
	}

	if (Input::GetKeyDown(KeyCode::Q))
	{
		if (m_Window->get_render_multiplier() > 0.1f)
		{
			m_Window->set_render_multiplier(m_Window->get_render_multiplier() - 0.1f);
		}
	}

	if (Input::GetKeyDown(KeyCode::E))
	{
		m_Window->set_render_multiplier(m_Window->get_render_multiplier() + 0.1f);
	}
}
