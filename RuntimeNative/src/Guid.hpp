#pragma once

#include "Core.hpp"

#include <string>

namespace leopph {
	class Guid {
		u64 data0;
		u64 data1;

	public:
		[[nodiscard]] LEOPPHAPI static auto Generate() -> Guid;
		[[nodiscard]] LEOPPHAPI static auto Parse(std::string_view str) -> Guid;

		[[nodiscard]] LEOPPHAPI auto ToString() const -> std::string;
		[[nodiscard]] LEOPPHAPI auto operator==(Guid const& other) const -> bool;
	};
}
