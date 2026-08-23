#include "Object.hpp"

#include "app.hpp"
#include "object_registry.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Object>{"Object"}
    .property("name", &sorcery::Object::name_);
}


namespace sorcery {
Object::Object() :
  id_{App::Instance().GetObjectRegistry().Register(ObserverPtr{this})} {}


Object::~Object() {
  App::Instance().GetObjectRegistry().Unregister(id_);
}


auto Object::GetName() const noexcept -> std::string const& {
  return name_;
}


auto Object::SetName(std::string const& name) -> void {
  name_ = name;
}


auto Object::GetId() const -> ObjectId const& {
  return id_;
}
}
