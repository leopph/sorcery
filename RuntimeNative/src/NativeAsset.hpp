#pragma once

#include "Object.hpp"

#include <cstdint>
#include <vector>

namespace leopph {
class NativeAsset : public Object {
public:
	LEOPPHAPI virtual auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void = 0;
};
}
