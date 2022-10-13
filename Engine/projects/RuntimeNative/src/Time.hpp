#pragma once

#include "Core.hpp"

namespace leopph
{
	LEOPPHAPI void init_time();
	LEOPPHAPI void measure_time();

	namespace managedbindings
	{
		f32 get_full_time();
		f32 get_frame_time();
	}
}