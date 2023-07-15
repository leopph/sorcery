#include "Resource.hpp"


namespace sorcery {
auto Resource::GetGuid() const -> Guid const& {
  return mGuid;
}


auto Resource::SetGuid(Guid const& guid) -> void {
  if (!guid.IsValid()) {
    throw std::runtime_error{ "Cannot set invalid Guid on Object." };
  }

  mGuid = guid;
}
}
