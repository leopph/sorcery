#include "random.hpp"


namespace sorcery {
SplitMix64::SplitMix64(std::uint64_t const seed) :
  state_{seed} {}


auto SplitMix64::operator()() -> std::uint64_t {
  auto result{(state_ += 0x9E3779B97f4A7C15)};
  result = (result ^ result >> 30) * 0xBF58476D1CE4E5B9;
  result = (result ^ result >> 27) * 0x94D049BB133111EB;
  return result ^ result >> 31;
}


Xorshift64::Xorshift64(std::uint64_t const seed) :
  state_{SplitMix64{seed}()} {}


auto Xorshift64::operator()() -> std::uint64_t {
  state_ ^= state_ << 7;
  state_ ^= state_ >> 9;
  return state_;
}


auto R2Sequence2d(std::size_t const idx) -> std::pair<float, float> {
  // https://observablehq.com/@jrus/plastic-sequence
  // https://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

  constexpr auto p1{0.7548776662466927}; // inverse of the plastic number
  constexpr auto p2{0.5698402909980532};
  return std::make_pair(
    static_cast<float>(std::fmod(p1 * static_cast<double>(idx), 1.0)),
    static_cast<float>(std::fmod(p2 * static_cast<double>(idx), 1.0))
  );
}


auto HaltonSequence(std::size_t idx, std::size_t const base) -> float {
  // https://en.wikipedia.org/wiki/Halton_sequence

  auto f{1.0f};
  auto r{0.0f};

  while (idx > 0) {
    f /= static_cast<float>(base);
    r = r + f * static_cast<float>(idx % base);
    idx = static_cast<std::size_t>(floorf(static_cast<float>(idx) / static_cast<float>(base)));
  }

  return r;
}
}
