#pragma once

#include <cstdint>
#include <span>
#include <vector>


namespace sorcery {
enum class CompressionError {
  None,
  Inconsistency,
  Unknown
};


auto Compress(std::span<std::uint8_t> in, std::vector<std::uint8_t>& out) -> CompressionError;
auto Uncompress(std::span<std::uint8_t const> in, std::uint64_t uncompressedSize, std::vector<std::uint8_t>& out) -> CompressionError;
}
