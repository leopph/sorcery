#include "resource_ref.hpp"

#include "app.hpp"
#include "resource_manager.hpp"


namespace sorcery::detail {
auto ResolveResource(ResourceId const& id) -> ObjectPtr<Resource> {
  return App::Instance().GetResourceManager().Resolve(id);
}
}
