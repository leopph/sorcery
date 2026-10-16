#pragma once

#include <cstdint>


namespace sorcery {
enum class CpuResidencyPolicy : std::uint8_t {
  kKeepResident,
  kReleaseAfterUpload
};
}
