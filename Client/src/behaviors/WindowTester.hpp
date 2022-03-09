#pragma once

#include "Leopph.hpp"


class WindowTester : public leopph::Behavior
{
	public:
		WindowTester();

		auto OnFrameUpdate() -> void override;

	private:
		leopph::Window* m_Window;
};
