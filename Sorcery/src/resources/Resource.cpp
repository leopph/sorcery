#include "Resource.hpp"


namespace sorcery {
auto Resource::GetId() const noexcept -> ResourceId const& {
  return id_;
}


auto Resource::SetId(ResourceId const& res_id) -> void {
  id_ = res_id;
}
}
