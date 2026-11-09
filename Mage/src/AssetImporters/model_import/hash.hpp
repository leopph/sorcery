#pragma once

#include <cstddef>
#include <cstdint>
#include <span>


namespace sorcery::mage::model_import {
[[nodiscard]] constexpr
auto Fnv1a64(std::span<std::byte const> bytes) noexcept -> std::uint64_t;


// SplitMix64 finalizer: a bijective 64-bit mixer with strong avalanche.
[[nodiscard]] constexpr
auto Mix64(std::uint64_t x) noexcept -> std::uint64_t;
}


#include "hash.inl"
