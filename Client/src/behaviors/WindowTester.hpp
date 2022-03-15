#pragma once

#include "Leopph.hpp"

#include <vector>


class WindowTester final : public leopph::Behavior
{
	public:
		WindowTester();
		auto OnFrameUpdate() -> void override;

	private:
		leopph::Window* m_Window;
		std::vector<leopph::Window::DisplayMode> m_DisplayModes;
};
