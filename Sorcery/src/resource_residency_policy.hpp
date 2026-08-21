#pragma once

#include <cstdint>


namespace sorcery {
enum class GpuResidencyPolicy : std::uint8_t {
  kDeferUpload,
  kMakeResident
};


enum class CpuResidencyPolicy : std::uint8_t {
  kKeepResident,
  kReleaseAfterUpload
};


struct ResourceResidencyPolicy {
  GpuResidencyPolicy gpu{GpuResidencyPolicy::kMakeResident};
  CpuResidencyPolicy cpu{CpuResidencyPolicy::kReleaseAfterUpload};
};
}
