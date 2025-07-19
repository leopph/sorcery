#include "resource_id.hpp"


namespace sorcery {
ResourceId::ResourceId(Guid const guid, int const idx_in_file) :
  guid_{guid},
  idx_in_file_{idx_in_file} {}


auto ResourceId::GetGuid() const noexcept -> Guid {
  return guid_;
}


auto ResourceId::GetIdxInFile() const noexcept -> int {
  return idx_in_file_;
}


auto ResourceId::IsValid() const noexcept -> bool {
  return guid_.IsValid() && idx_in_file_ >= 0;
}


auto operator<=>(ResourceId const& lhs, ResourceId const& rhs) noexcept -> std::strong_ordering {
  return lhs.GetGuid() <=> rhs.GetGuid() != std::strong_ordering::equivalent
           ? lhs.GetGuid() <=> rhs.GetGuid()
           : lhs.GetIdxInFile() <=> rhs.GetIdxInFile();
}
}
