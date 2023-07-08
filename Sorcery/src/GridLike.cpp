#include "GridLike.hpp"


namespace sorcery {
auto GridLike::ThrowIfSubdivInvalid() const -> void {
  if (!IsPowerOfTwo(mSubdivSize)) {
    throw std::runtime_error{ "GridLike subdivision size must be power of 2." };
  }
}


auto GridLike::ThrowIfIndexIsInvalid(int const idx) const -> void {
  if (idx < 0 || idx >= GetElementCount()) {
    throw std::runtime_error{ "Invalid GridLike element index." };
  }
}


auto GridLike::SetSubdivisionSize(int const subdivSize) -> void {
  mSubdivSize = subdivSize;
  ThrowIfSubdivInvalid();
}


GridLike::GridLike(int const subdivSize):
  mSubdivSize{ subdivSize } {
  ThrowIfSubdivInvalid();
}


auto GridLike::GetSubdivisionSize() const noexcept -> int {
  return mSubdivSize;
}


auto GridLike::GetElementCount() const noexcept -> int {
  return mSubdivSize * mSubdivSize;
}


auto GridLike::GetNormalizedElementSize() const noexcept -> float {
  return 1.0f / static_cast<float>(mSubdivSize);
}


auto GridLike::GetNormalizedElementOffset(int const idx) const -> Vector2 {
  ThrowIfIndexIsInvalid(idx);

  Vector2 offset{ GetNormalizedElementSize() };
  offset[0] *= static_cast<float>(static_cast<int>(idx % mSubdivSize));
  offset[1] *= static_cast<float>(static_cast<int>(idx / mSubdivSize));

  return offset;
}
}
