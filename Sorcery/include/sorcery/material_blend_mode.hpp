#pragma once

#include <cstdint>


namespace sorcery {
enum class MaterialBlendMode : std::uint8_t {
  kOpaque,
  kAlphaClip
};
}
