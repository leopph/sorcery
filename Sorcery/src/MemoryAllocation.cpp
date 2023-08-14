#include "MemoryAllocation.hpp"


namespace sorcery {
namespace {
LinearMemoryResource gLinMemRes{static_cast<std::size_t>(128) * 1024};
}


auto GetTmpMemRes() noexcept -> LinearMemoryResource& {
  return gLinMemRes;
}
}
