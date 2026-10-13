#pragma once

#include <cstdint>


namespace sorcery::rendering {
struct SsaoParams {
  float radius;
  float bias;
  float power;
  std::uint32_t sample_count;
};
}
