#pragma once

#include "Leopph.hpp"


class Exiter final : public leopph::BehaviorNode
{
	public:
		void on_frame_update() override;
};
