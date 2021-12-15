#pragma once

#include "Leopph.hpp"


class WindowTester : public leopph::Behavior
{
	public:
		WindowTester(leopph::Entity* entity);

		void OnFrameUpdate() override;
};