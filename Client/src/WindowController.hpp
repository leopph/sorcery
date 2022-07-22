#pragma once

#include "Leopph.hpp"

#include <vector>


class WindowController final : public leopph::Behavior
{
	public:
		WindowController();
		void OnFrameUpdate() override;

	private:
		leopph::Window* m_Window;
		std::vector<leopph::Window::DisplayMode> m_DisplayModes;
};
