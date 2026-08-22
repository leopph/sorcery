#include "Resource.hpp"


namespace sorcery {
auto Resource::GetResId() const noexcept -> ResourceId const& {
  return res_id_;
}


auto Resource::SetResId(ResourceId const& res_id) -> void {
  res_id_ = res_id;
}
}
