#include "object_ptr.hpp"

#include "app.hpp"
#include "object_registry.hpp"


namespace sorcery::detail {
auto ResolveObject(ObjectId const id) -> ObserverPtr<Object> {
  return App::Instance().GetObjectRegistry().Resolve(id);
}
}
