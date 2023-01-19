#pragma once

#include "Core.hpp"

namespace leopph {
	LEOPPHAPI auto init_time() -> void;
	LEOPPHAPI auto measure_time() -> void;

	LEOPPHAPI [[nodiscard]] auto get_full_time() noexcept -> f32;
	LEOPPHAPI [[nodiscard]] auto get_frame_time() noexcept -> f32;
}
