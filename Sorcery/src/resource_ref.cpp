#include "resource_ref.hpp"

#include "app.hpp"
#include "resource_manager.hpp"


namespace sorcery::detail {
auto ResolveResource(ResourceId const& id) -> ObserverPtr<Resource> {
  return MakeObserver(App::Instance().GetResourceManager().GetOrLoad(id));
}
}
