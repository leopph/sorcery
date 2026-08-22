#pragma once

#include <cstdint>
#include <limits>


namespace sorcery {
struct ObjectId {
  static constexpr std::uint32_t kInvalidIndex{std::numeric_limits<std::uint32_t>::max()};

  std::uint32_t idx{kInvalidIndex};
  std::uint32_t gen{0};


  [[nodiscard]] constexpr
  auto IsValid() const noexcept -> bool { return idx != kInvalidIndex; }


  auto operator<=>(ObjectId const&) const = default;
};
}
