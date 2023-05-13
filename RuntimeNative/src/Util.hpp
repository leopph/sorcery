#pragma once

#include "Core.hpp"

#include <algorithm>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "Math.hpp"


namespace leopph {
template<typename T>
struct Extent2D {
  T width;
  T height;
};


template<typename T>
struct Point2D {
  T x;
  T y;
};


struct Viewport {
  Point2D<unsigned> position;
  Extent2D<unsigned> extent;
};


struct NormalizedViewport {
  Point2D<float> position;
  Extent2D<float> extent;
};


template<std::integral To, std::integral From>
[[nodiscard]] constexpr auto clamp_cast(From const what) -> To {
  if constexpr (std::cmp_less_equal(std::numeric_limits<To>::min(), std::numeric_limits<From>::min())) {
    if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max())) {
      return static_cast<To>(what);
    } else {
      return static_cast<To>(std::min<From>(what, std::numeric_limits<To>::max()));
    }
  } else {
    if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max())) {
      return static_cast<To>(std::max<From>(what, std::numeric_limits<To>::min()));
    } else {
      return static_cast<To>(std::clamp<From>(what, std::numeric_limits<To>::min(), std::numeric_limits<To>::max()));
    }
  }
}


template<auto& Obj, auto MemberFunc, typename... Args>
auto Call(Args&&... args) noexcept(std::is_nothrow_invocable_v<decltype(MemberFunc), decltype(Obj), Args...>) {
  return (Obj.*MemberFunc)(std::forward<Args>(args)...);
}


template<typename T>
using ObserverPtr = T*;


template<class T>
concept Scalar = std::is_scalar_v<T>;


[[nodiscard]] constexpr auto RoundToNextMultiple(auto const what, auto const multipleOf) {
  if (multipleOf == 0) {
    return what;
  }

  auto const remainder{ what % multipleOf };

  if (remainder == 0) {
    return what;
  }

  return what + multipleOf - remainder;
}


[[nodiscard]] LEOPPHAPI auto Contains(std::string_view src, std::string_view target) -> bool;

LEOPPHAPI auto CalculateNormals(std::span<Vector3 const> positions, std::span<unsigned const> indices, std::vector<Vector3>& out) -> std::vector<Vector3>&;
LEOPPHAPI auto CalculateTangents(std::span<Vector3 const> positions, std::span<Vector2 const> uvs, std::span<unsigned const> indices, std::vector<Vector3>& out) -> void;

// Appends an index to the specified file path to avoid name clashes
[[nodiscard]] LEOPPHAPI auto IndexFileNameIfNeeded(std::filesystem::path const& filePathAbsolute) -> std::filesystem::path;

[[nodiscard]] LEOPPHAPI auto Join(std::span<std::string const> strings, std::string const& delim) -> std::string;
}
