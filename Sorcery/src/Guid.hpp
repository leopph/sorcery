#pragma once

#include "Core.hpp"

#include <compare>
#include <string>


namespace sorcery {
class Guid {
  u64 mDataLo{ 0 };
  u64 mDataHi{ 0 };

public:
  [[nodiscard]] LEOPPHAPI static auto Invalid() noexcept -> Guid;

  Guid() = default;
  LEOPPHAPI Guid(u64 data0, u64 data1);

  [[nodiscard]] LEOPPHAPI static auto Generate() -> Guid;
  [[nodiscard]] LEOPPHAPI static auto Parse(std::string_view str) -> Guid;

  [[nodiscard]] LEOPPHAPI auto ToString() const -> std::string;
  [[nodiscard]] LEOPPHAPI auto operator<=>(Guid const& other) const noexcept -> std::strong_ordering;

  [[nodiscard]] LEOPPHAPI auto IsValid() const noexcept -> bool;

  [[nodiscard]] LEOPPHAPI explicit operator std::string() const;
};
}
