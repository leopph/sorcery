#pragma once

#include "Object.hpp"

#include <cstdint>
#include <vector>


namespace sorcery {
class NativeAsset : public Object {
  RTTR_ENABLE(Object)
public:
  LEOPPHAPI virtual auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void = 0;
};
}
