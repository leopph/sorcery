#pragma once

#include "Core.hpp"

#include <string>


namespace sorcery {
class Guid {
  u64 mData0{ 0 };
  u64 mData1{ 0 };

public:
  Guid() = default;
  LEOPPHAPI Guid(u64 data0, u64 data1);

  [[nodiscard]] LEOPPHAPI static auto Generate() -> Guid;
  [[nodiscard]] LEOPPHAPI static auto Parse(std::string_view str) -> Guid;

  [[nodiscard]] LEOPPHAPI auto ToString() const -> std::string;
  [[nodiscard]] LEOPPHAPI auto operator==(Guid const& other) const -> bool;

  [[nodiscard]] LEOPPHAPI auto IsValid() const noexcept -> bool;
};
}
