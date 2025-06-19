#pragma once

#include "Core.hpp"

#include <cstdint>
#include <random>
#include <utility>


namespace sorcery {
class SplitMix64 {
public:
  LEOPPHAPI explicit SplitMix64(std::uint64_t seed = std::random_device{}());
  LEOPPHAPI auto operator()() -> std::uint64_t;

private:
  std::uint64_t state_;
};


// https://en.wikipedia.org/wiki/Xorshift
class Xorshift64 {
public:
  LEOPPHAPI explicit Xorshift64(std::uint64_t seed = std::random_device{}());
  LEOPPHAPI auto operator()() -> std::uint64_t;

private:
  std::uint64_t state_;
};


[[nodiscard]] auto PlasticSequence2d(std::size_t idx) -> std::pair<float, float>;
[[nodiscard]] auto HaltonSequence(std::size_t idx, std::size_t base) -> float;
}
