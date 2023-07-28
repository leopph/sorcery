#include "Object.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Object>{ "Object" }
    .property("name", &sorcery::Object::mName);
}


namespace sorcery {
std::vector<ObserverPtr<Object>> Object::sAllObjects;


Object::Object() {
  sAllObjects.emplace_back(this);
}


Object::~Object() {
  std::erase(sAllObjects, this);
}


auto Object::GetName() const noexcept -> std::string_view {
  return mName;
}


auto Object::SetName(std::string name) noexcept -> void {
  mName = std::move(name);
}


auto Object::Destroy(Object const& obj) -> void {
  auto const p{ std::addressof(obj) };
  delete p;

  for (auto const otherObj : sAllObjects) {
    for (auto const prop : rttr::type::get(*otherObj).get_properties()) {
      if (prop.get_type().is_pointer()) {
        if (prop.get_value(*otherObj).get_value<void*>() == p) {
          prop.set_value(*otherObj, nullptr);
        }
      }
    }
  }
}
}
