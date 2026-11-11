#pragma once

#include <cstdint>

#include <dxgiformat.h>

#include <sorcery/Core.hpp>


namespace sorcery::rendering {
[[nodiscard]] SORCERYAPI
auto GetFormatChannelCount(DXGI_FORMAT format) -> std::uint32_t;
}
