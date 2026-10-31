#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>

#include <sorcery/Core.hpp>


namespace sorcery {
class WorldEntityGroup {
public:
  WorldEntityGroup() = default;
  SORCERYAPI explicit WorldEntityGroup(std::uint32_t value) noexcept;

  [[nodiscard]] SORCERYAPI
  auto IsNone() const noexcept -> bool;

  [[nodiscard]] SORCERYAPI
  auto Value() const noexcept -> std::uint32_t;

  SORCERYAPI explicit operator std::uint32_t() const noexcept;

  auto operator<=>(WorldEntityGroup const& other) const noexcept = default;

private:
  std::uint32_t value_{0};
};
}


template<>
struct std::hash<sorcery::WorldEntityGroup> {
  [[nodiscard]] auto operator()(sorcery::WorldEntityGroup const& group) const noexcept -> std::size_t;
};


#include <sorcery/world_entity_group.inl>
