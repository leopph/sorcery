#pragma once

namespace sorcery::mage::model_import {
constexpr
auto Fnv1a64(std::span<std::byte const> const bytes) noexcept -> std::uint64_t {
  auto hash = 0xcbf29ce484222325ULL;

  for (auto const byte : bytes) {
    hash ^= static_cast<std::uint64_t>(byte);
    hash *= 0x100000001b3ULL;
  }

  return hash;
}


constexpr
auto Mix64(std::uint64_t x) noexcept -> std::uint64_t {
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9ULL;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebULL;
  x ^= x >> 31;
  return x;
}
}
