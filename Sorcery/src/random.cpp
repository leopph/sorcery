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
}
