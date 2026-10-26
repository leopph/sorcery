#pragma once

#include <compare>
#include <cstdint>

#include <sorcery/Core.hpp>


namespace sorcery {
class SceneInstanceId {
public:
  SceneInstanceId() = default;
  SORCERYAPI explicit SceneInstanceId(std::uint32_t value) noexcept;

  [[nodiscard]] SORCERYAPI
  auto HasScene() const noexcept -> bool;

  [[nodiscard]] SORCERYAPI
  auto Value() const noexcept -> std::uint32_t;

  SORCERYAPI explicit operator std::uint32_t() const noexcept;

  auto operator<=>(SceneInstanceId const& other) const noexcept = default;

private:
  std::uint32_t value_{0};
};
}
