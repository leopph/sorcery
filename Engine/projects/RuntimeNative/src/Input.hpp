#pragma once

#include "Core.hpp"

namespace leopph
{
	LEOPPHAPI void update_keyboard_state();

	namespace managedbindings
	{
		bool get_key(u8 key);
		bool get_key_down(u8 key);
		bool get_key_up(u8 key);
	}
}