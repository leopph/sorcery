#pragma once

#include <cstdint>


namespace sorcery {
struct ObjectId {
  std::uint32_t idx;
  std::uint32_t gen;

  auto operator<=>(ObjectId const&) const = default;
};
}
