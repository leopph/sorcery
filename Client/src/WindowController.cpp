#include "WindowController.hpp"

#include "Context.hpp"

using leopph::Input;
using leopph::KeyCode;
using leopph::Window;


WindowController::WindowController() :
	mWindow{leopph::get_main_window()},
	mDisplayModes
	{
		[this]
		{
			std::vector<Window::DisplayMode> displayModes;
			for (auto const& displayMode : mWindow.get_supported_display_modes())
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
		mWindow.set_fullscreen(!mWindow.is_fullscreen());
	}

	if (Input::GetKeyDown(KeyCode::V))
	{
		mWindow.set_vsync(!mWindow.get_vsync());
	}

	if (Input::GetKeyDown(KeyCode::One) && !mDisplayModes.empty())
	{
		mWindow.set_width(mDisplayModes[0].width);
		mWindow.set_height(mDisplayModes[0].height);
	}

	if (Input::GetKeyDown(KeyCode::Two) && mDisplayModes.size() > 1)
	{
		mWindow.set_width(mDisplayModes[1].width);
		mWindow.set_height(mDisplayModes[1].height);
	}

	if (Input::GetKeyDown(KeyCode::Three) && mDisplayModes.size() > 2)
	{
		mWindow.set_width(mDisplayModes[2].width);
		mWindow.set_height(mDisplayModes[2].height);
	}

	if (Input::GetKeyDown(KeyCode::Four) && mDisplayModes.size() > 3)
	{
		mWindow.set_width(mDisplayModes[3].width);
		mWindow.set_height(mDisplayModes[3].height);
	}

	if (Input::GetKeyDown(KeyCode::Five) && mDisplayModes.size() > 4)
	{
		mWindow.set_width(mDisplayModes[4].width);
		mWindow.set_height(mDisplayModes[4].height);
	}

	/*if (Input::GetKeyDown(KeyCode::Q))
	{
		if (mWindow.get_render_multiplier() > 0.1f)
		{
			mWindow.set_render_multiplier(mWindow.get_render_multiplier() - 0.1f);
		}
	}

	if (Input::GetKeyDown(KeyCode::E))
	{
		mWindow.set_render_multiplier(mWindow.get_render_multiplier() + 0.1f);
	}*/
}
