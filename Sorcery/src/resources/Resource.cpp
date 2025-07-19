#include "Resource.hpp"


namespace sorcery {
auto Resource::GetId() const noexcept -> ResourceId const& {
  return id_;
}


auto Resource::SetId(ResourceId const& res_id) -> void {
  if (!res_id.IsValid()) {
    throw std::runtime_error{"Cannot set invalid ResourceId on Resource."};
  }

  id_ = res_id;
}
}
