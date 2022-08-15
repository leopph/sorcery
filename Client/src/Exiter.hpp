#pragma once

#include "Leopph.hpp"


class Exiter final : public leopph::Behavior
{
	public:
		void on_frame_update() override;
};
