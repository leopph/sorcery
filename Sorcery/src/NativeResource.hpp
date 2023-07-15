#pragma once

#include "Resource.hpp"

#include <cstdint>
#include <vector>


namespace sorcery {
class NativeResource : public Resource {
  RTTR_ENABLE(Resource)
public:
  LEOPPHAPI virtual auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void = 0;
};
}
