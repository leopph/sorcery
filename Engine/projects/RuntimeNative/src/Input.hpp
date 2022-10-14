#pragma once

#include "Core.hpp"
#include "Util.hpp"

namespace leopph
{
	LEOPPHAPI [[nodiscard]] bool init_input_system();
	LEOPPHAPI [[nodiscard]] bool update_input_system();
	LEOPPHAPI void cleanup_input_system();

	namespace managedbindings
	{
		bool get_key(u8 key);
		bool get_key_down(u8 key);
		bool get_key_up(u8 key);
		Point2DI32 get_mouse_pos();
		Point2DI32 get_mouse_delta();
	}
}