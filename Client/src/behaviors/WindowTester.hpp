#pragma once

#include "Leopph.hpp"


class WindowTester : public leopph::Behavior
{
	public:
		using Behavior::Behavior;

		void OnFrameUpdate() override;
};