#include "sorcery/object_ptr.hpp"

#include "sorcery/host/app.hpp"
#include "sorcery/host/object_registry.hpp"


namespace sorcery::detail {
auto ResolveObject(ObjectId const id) -> ObserverPtr<Object> {
  return App::Instance().GetObjectRegistry().Resolve(id);
}
}
