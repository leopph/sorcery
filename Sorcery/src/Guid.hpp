#pragma once

#include "Core.hpp"

#include <cstdint>
#include <compare>
#include <string>


namespace sorcery {
class Guid {
  std::uint64_t mLowBits{0};
  std::uint64_t mHighBits{0};

public:
  [[nodiscard]] LEOPPHAPI static auto Invalid() noexcept -> Guid;

  Guid() = default;
  LEOPPHAPI Guid(std::uint64_t lowBits, std::uint64_t highBits);

  [[nodiscard]] LEOPPHAPI static auto Generate() -> Guid;
  [[nodiscard]] LEOPPHAPI static auto Parse(std::string_view str) -> Guid;

  [[nodiscard]] LEOPPHAPI auto ToString() const -> std::string;
  [[nodiscard]] LEOPPHAPI auto operator<=>(Guid const& other) const noexcept -> std::strong_ordering;
  [[nodiscard]] auto operator==(Guid const& other) const noexcept -> bool = default;

  [[nodiscard]] LEOPPHAPI auto IsValid() const noexcept -> bool;

  [[nodiscard]] LEOPPHAPI explicit operator std::string() const;
};
}
