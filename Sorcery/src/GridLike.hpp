#pragma once
#include "Math.hpp"


namespace sorcery {
class GridLike {
  int mSubdivSize;

  auto ThrowIfSubdivInvalid() const -> void;

protected:
  auto ThrowIfIndexIsInvalid(int idx) const -> void;
  auto SetSubdivisionSize(int subdivSize) -> void;

public:
  explicit GridLike(int subdivSize);

  // The grid has N*N cells, this is the N of that.
  [[nodiscard]] auto GetSubdivisionSize() const noexcept -> int;
  [[nodiscard]] auto GetElementCount() const noexcept -> int;
  [[nodiscard]] auto GetNormalizedElementSize() const noexcept -> float;
  [[nodiscard]] auto GetNormalizedElementOffset(int idx) const -> Vector2;
};
}
