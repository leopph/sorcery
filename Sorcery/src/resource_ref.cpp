#include "sorcery/resource_ref.hpp"

#include "sorcery/host/app.hpp"
#include "sorcery/host/assets/resource_manager.hpp"


namespace sorcery::detail {
auto ResolveResource(ResourceId const& id) -> ObjectPtr<Resource> {
  return App::Instance().GetResourceManager().Resolve(id);
}
}
