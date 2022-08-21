#pragma once

#include "Leopph.hpp"

#include <vector>


class WindowController final : public leopph::Behavior
{
	public:
		WindowController();
		void on_frame_update() override;

	private:
		leopph::Window& mWindow;
		std::vector<leopph::Window::DisplayMode> mDisplayModes;
};
