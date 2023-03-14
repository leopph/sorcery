#pragma once

#include "Object.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace leopph {
class NativeAsset : public Object {
public:
	LEOPPHAPI virtual auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void = 0;
	LEOPPHAPI virtual auto Deserialize(std::span<std::uint8_t const> bytes) -> void = 0;
};
}
